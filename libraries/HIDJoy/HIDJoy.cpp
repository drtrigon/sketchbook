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

size_t HIDJoy::writeGame(int8_t Lx, int8_t Ly, int8_t Rx, int8_t Ry)
{
  while(!usbInterruptIsReady()) {
    usbPoll();
  }

  gameReport.left_x = Lx;
  gameReport.left_y = Ly;
  gameReport.right_x = Rx;
  gameReport.right_y = Ry;

  usbSetInterrupt((unsigned char *)&gameReport, sizeof(gamepad_report_t));
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
  usbSetInterrupt(outBuffer, 8);
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
  return count;
}
