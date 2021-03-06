//This Modifed version of VUSB was created in part by Rickey Ward, the USB HID descriptor
//was designed to be generic and useful, feel free to use this code to develop cool
//arduino based gamepads!

#include "HIDJoy.h"

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */
#include <avr/eeprom.h>
#include <avr/pgmspace.h>   /* required by usbdrv.h */
#include "usbdrv.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

static uchar received = 0;
static uchar outBuffer[8];
static uchar inBuffer[HIDSERIAL_INBUFFER_SIZE];
static uchar reportId = 0;
static uchar bytesRemaining;
static uchar* pos;

static gamepad_report_t gameReport;

/*
PROGMEM const char usbHidReportDescriptor[36] = {
0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
0x09, 0x05,                    // USAGE (Game Pad)
0xa1, 0x01,                    // COLLECTION (Application)
0xa1, 0x00,                    //   COLLECTION (Physical)
0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
0x09, 0x30,                    //     USAGE (X)
0x09, 0x31,                    //     USAGE (Y)
0x09, 0x32,                    //     USAGE (Z) rx
0x09, 0x35,                    //     USAGE (Rx) ry
0x35, 0x00,                    //     PHYSICAL_MINIMUM (0)
0x46, 0xff, 0x00,              //     PHYSICAL_MAXIMUM (255)
0x15, 0x00,                    //     LOGICAL_MINIMUM (-127)
0x26, 0xff, 0x00,                    //     LOGICAL_MAXIMUM (127)
0x75, 0x08,                    //     REPORT_SIZE (8)
0x95, 0x04,                    //     REPORT_COUNT (4)
0x81, 0x02,                    //     INPUT (Data,Var,Abs)
0xc0,                          //   END_COLLECTION
0xc0                           // END_COLLECTION
};
*/
PROGMEM const char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = {
0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
0x09, 0x05,                    // USAGE (Game Pad)
0xa1, 0x01,                    // COLLECTION (Application)
0xa1, 0x00,                    //   COLLECTION (Physical)
0x05, 0x09,                    //     USAGE_PAGE (Button)
0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
0x29, 0x08,                    //     USAGE_MAXIMUM (Button 8)
0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
0x95, 0x08,                    //     REPORT_COUNT (8)
0x75, 0x01,                    //     REPORT_SIZE (1)
0x81, 0x02,                    //     INPUT (Data,Var,Abs)
0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
0x09, 0x30,                    //     USAGE (X)
0x09, 0x31,                    //     USAGE (Y)
0x09, 0x32,                    //     USAGE (Z) rx
0x09, 0x33,                    //     USAGE (Rx)
0x09, 0x34,                    //     USAGE (Ry)
0x09, 0x35,                    //     USAGE (Rz)
0x09, 0x36,                    //     USAGE (Slider)
0x09, 0x36,                    //     USAGE (Slider)
0x35, 0x00,                    //     PHYSICAL_MINIMUM (0)
0x46, 0xff, 0x00,              //     PHYSICAL_MAXIMUM (255)
0x15, 0x00,                    //     LOGICAL_MINIMUM (-127)
0x26, 0xff, 0x00,              //     LOGICAL_MAXIMUM (127)
0x75, 0x08,                    //     REPORT_SIZE (8)
0x95, 0x08,                    //     REPORT_COUNT (8)
0x81, 0x02,                    //     INPUT (Data,Var,Abs)
0xc0,                          //   END_COLLECTION
0xc0                           // END_COLLECTION
};
// see also http://eleccelerator.com/tutorial-about-usb-hid-report-descriptors/
// see also https://forum.pjrc.com/threads/23681-Many-axis-joystick/page2

/* usbFunctionRead() is called when the host requests a chunk of data from
 * the device. For more information see the documentation in usbdrv/usbdrv.h.
 */
uchar   usbFunctionRead(uchar *data, uchar len)
{
    return 0;
}

/* usbFunctionWrite() is called when the host sends a chunk of data to the
 * device. For more information see the documentation in usbdrv/usbdrv.h.
 */
uchar   usbFunctionWrite(uchar *data, uchar len)
{
    if (reportId == 0) {
        int i;
        if(len > bytesRemaining)
            len = bytesRemaining;
        bytesRemaining -= len;
        //int start = (pos==inBuffer)?1:0;
        for(i=0;i<len;i++) {
            if (data[i]!=0) {
                *pos++ = data[i];
             }
        }
        if (bytesRemaining == 0) {
            received = 1;
            *pos++ = 0;
            return 1;
        } else {
            return 0;
        }
    } else {
        return 1;
    }
}

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
    usbRequest_t    *rq = (usbRequest_t *)data;
    reportId = rq->wValue.bytes[0];
    if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    /* HID class request */
        if(rq->bRequest == USBRQ_HID_GET_REPORT){
          /* wValue: ReportType (highbyte), ReportID (lowbyte) */
            /* since we have only one report type, we can ignore the report-ID */
            return USB_NO_MSG;  /* use usbFunctionRead() to obtain data */
        }else if(rq->bRequest == USBRQ_HID_SET_REPORT){
            /* since we have only one report type, we can ignore the report-ID */
            pos = inBuffer;
            bytesRemaining = rq->wLength.word;
            if(bytesRemaining > sizeof(inBuffer))
                bytesRemaining = sizeof(inBuffer);
            return USB_NO_MSG;  /* use usbFunctionWrite() to receive data from host */
        }
    }else{
        /* ignore vendor type requests, we don't use any */
    }
    return 0;
}

HIDJoy::HIDJoy()
{

}

void HIDJoy::begin()
{
  uchar i;
  cli();
  usbDeviceDisconnect();
  i = 0;
  while(--i){             /* fake USB disconnect for > 250 ms */
    _delay_ms(1);
  }
  usbDeviceConnect();
  usbInit();
  sei();

  received = 0;
}

void HIDJoy::poll()
{
    usbPoll();
}

uchar HIDJoy::available()
{
    return received;
}

uchar HIDJoy::read(uchar *buffer)
{
    if(received == 0) return 0;
    int i;
    for(i=0;inBuffer[i]!=0&&i<HIDSERIAL_INBUFFER_SIZE;i++)
    {
        buffer[i] = inBuffer[i];
    }
    inBuffer[0] = 0;
    buffer[i] = 0;
    received = 0;
    return i;
}

// write one character

size_t HIDJoy::writeGame(int8_t Lx, int8_t Ly, int8_t Rx, int8_t Ry, int8_t ch5, int8_t ch6, int8_t ch7, int8_t ch8)
{
  while(!usbInterruptIsReady()) {
    usbPoll();
  }

  gameReport.buttons = (Lx<0) | ((Ly<0) << 1) | ((Rx<0) << 2) | ((Ry<0) << 3) |
                       ((ch5<0) << 4) | ((ch6<0) << 5) | ((ch7<0) << 6) | ((ch8<0) << 7);
  gameReport.left_x = Lx;
  gameReport.left_y = Ly;
  gameReport.right_x = Rx;
  gameReport.right_y = Ry;
  gameReport.ch5 = ch5;
  gameReport.ch6 = ch6;
  gameReport.ch7 = ch7;
  gameReport.ch8 = ch8;

//  usbSetInterrupt((unsigned char *)&gameReport, sizeof(gamepad_report_t));  // write max 8 bytes
  write((uint8_t *)&gameReport, sizeof(gamepad_report_t));  // write more than 8 bytes
  return 1;
}


size_t HIDJoy::write(uint8_t data)
{
  while(!usbInterruptIsReady()) {
    usbPoll();
  }
  memset(outBuffer, 0, 8);
  outBuffer[0] = data;
  usbSetInterrupt(outBuffer, 8);
  return 1;
}

// You should be able to simulate a 16 byte interrupt transfer with the following procedure:
//   * Pass the first 8 bytes with usbSetInterrupt().
//   * Wait until they are transferred with usbInterruptIsReady().
//   * Pass the next 8 bytes with usbSetInterrupt().
//   * Wait until the transaction is complete with usbInterruptIsReady().
//   * Call usbSetInterrupt() with 0 bytes to indicate the end of transfer.
// If your payload is only 15 bytes, you save the last 0 byte transaction. The end of transfer is indicated by a transaction with less than 8 bytes.
// see also https://forums.obdev.at/viewtopic.php?p=330&sid=a371f0d9c43f177db4b8f5987f0e5fb2#p330

// write up to 8 characters
size_t HIDJoy::write8(const uint8_t *buffer, size_t size)
{
  unsigned char i;
  while(!usbInterruptIsReady()) {
    usbPoll();
  }
  memset(outBuffer, 0, 8);
  for(i=0;i<size && i<8; i++) {
    outBuffer[i] = buffer[i];
  }
//  usbSetInterrupt(outBuffer, 8);
  usbSetInterrupt(outBuffer, (size<8) ? size : 8);
  return (i);
}

// write a string
size_t HIDJoy::write(const uint8_t *buffer, size_t size)
{
  size_t count = 0;
  unsigned char i;
  for(i=0; i< (size/8) + 1; i++) {
    count += write8(buffer+i*8, (size<(count+8)) ? (size-count) : 8);
  }
// also need to call usbSetInterrupt() with 0 bytes to indicate the end of transfer
// if size is a multiple of 8 (8 excluded; 16, 24, 32, ...)
  return count;
}
