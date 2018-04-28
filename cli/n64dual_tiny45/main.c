/* Name: main.c
 * Project: N64 to USB converter
 * Author: Raphael Assenat <raph@raphnet.net
 * Copyright: (C) 2007 Raphael Assenat <raph@raphnet.net>
 * Modified by: Matthias Linder <matthias@matthiaslinder.com>, 2013
 *          + To work with two simultanous N64 controllers
 *          + To work on an auto-calibrated 16.5 MHz ATTiny45
 * License: Proprietary, free under certain conditions. See Documentation.
 * Comments: Based on HID-Test by Christian Starkjohann
 * Tabsize: 4
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <string.h>
#include <usbdrv.h>

#include "gamepad.h"
#include "n64.h"

#define LED (1 << PB0)

static Gamepad *curGamepad;

/* ----------------------- USB Descriptor ---------------------------------- */
PROGMEM const char usbHidReportDescriptor[94] PROGMEM = {
    //Controller 1
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x04,                    // USAGE (Joystick)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x85, 0x01,        		   //   REPORT_ID (1)
	
    0x09, 0x01,                    //   USAGE (Pointer)    
    0xa1, 0x00,                    //   COLLECTION (Physical)
    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x26, 0xFF, 0x00,              //     LOGICAL_MAXIMUM (255)
    0x75, 0x08,                    //     REPORT_SIZE (8) bits
    0x95, 0x02,                    //     REPORT_COUNT (2) times
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0xc0,                          //   END_COLLECTION (Physical),

    0x05, 0x09,                    //   USAGE_PAGE (Button)
    0x19, 0x01,                    //   USAGE_MINIMUM (Button 1)
    0x29, 0x0E,                    //   USAGE_MAXIMUM (Button 14)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x10,                    //   REPORT_COUNT (16)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)

    0xc0,                           // END_COLLECTION (Application)

    //Controller 2
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x04,                    // USAGE (Joystick)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x85, 0x02,         		   //   REPORT_ID (2)
	
    0x09, 0x01,                    //   USAGE (Pointer)    
    0xa1, 0x00,                    //   COLLECTION (Physical)
    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x26, 0xFF, 0x00,              //     LOGICAL_MAXIMUM (255)
    0x75, 0x08,                    //     REPORT_SIZE (8) bits
    0x95, 0x02,                    //     REPORT_COUNT (2) times
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0xc0,                          //   END_COLLECTION (Physical),

    0x05, 0x09,                    //   USAGE_PAGE (Button)
    0x19, 0x01,                    //   USAGE_MINIMUM (Button 1)
    0x29, 0x0E,                    //   USAGE_MAXIMUM (Button 14)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x10,                    //   REPORT_COUNT (16)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)

    0xc0                           // END_COLLECTION (Application)
};

static uchar    reportBuffer[8];    /* buffer for HID reports */

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

static uchar    idleRate;           /* in 4 ms units */

usbMsgLen_t	usbFunctionSetup(uchar data[8])
{
	usbRequest_t    *rq = (void *)data;
	usbMsgPtr = reportBuffer;
	if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    /* class request type */
		if(rq->bRequest == USBRQ_HID_GET_REPORT){  /* wValue: ReportType (highbyte), ReportID (lowbyte) */
			memset(reportBuffer, 0, sizeof(reportBuffer));
			/* we only have one report type, so don't look at wValue */
			if (rq->wValue.bytes[0] == 1)		//Buttons
			{
				curGamepad->buildReport(reportBuffer);
				//return curGamepad->report_size;
			}
			return USB_NO_MSG;
		} else if(rq->bRequest == USBRQ_HID_GET_IDLE) {
			usbMsgPtr = &idleRate;
			return 1;
		} else if(rq->bRequest == USBRQ_HID_SET_IDLE) {
			idleRate = rq->wValue.bytes[1];
		} else if (rq->bRequest == USBRQ_HID_SET_REPORT) {
			return USB_NO_MSG;
		}
	}
	return 0;
}
/* ------------------------------------------------------------------------- */

int main(void)
{
        wdt_reset();
        wdt_enable(WDTO_1S);

        // Setup LED
        DDRB |= LED;
        PORTB &= ~LED; // off
      
        usbInit();
        usbDeviceDisconnect();
   
        while(1)
	{
                wdt_reset();
                PORTB ^= LED;
		_delay_ms(50);

		/* Check for n64 controller */
		curGamepad = n64GetGamepad();
		curGamepad->init();
		if (curGamepad->probe()) // this will take ~400ms
			break;
	}

        PORTB &= ~LED;
	wdt_reset();
	curGamepad->init();
	usbDeviceConnect();
	sei();        

	char must_report = 0;
	uchar idleCounter = 0;
        uint8_t shutdownTicks = 0xFF;
	
	while (1) 
	{	/* main event loop */
		wdt_reset();

		// this must be called at each 10 ms or less
		usbPoll();

                // Report every 'idleRate*4' ms if idle
                if (idleCounter <= 1) {
                  must_report = 1; 
                } else {
                  idleCounter--; // 1ms
                }

                // Also report if there has been any activity
                // sleep_enable(); sleep_cpu(); sleep_disable(); _delay_us(100);
                curGamepad->update();
                if (curGamepad->changed()) {
                  must_report = 1;
                }

                // Do we need to report changes?
                if (must_report && usbInterruptIsReady()) {
                  must_report = 0;
                  idleCounter = idleRate;
                  curGamepad->buildReport(reportBuffer);

                  // Make the transfer & wait until everything is finished
                  usbSetInterrupt(reportBuffer, curGamepad->report_size);
                  shutdownTicks = 0xFF;
                  PORTB |= LED;
                  while (!usbInterruptIsReady() && --shutdownTicks != 0) { wdt_reset(); usbPoll(); _delay_us(200); }
                }
                else { _delay_ms(1); }
            
                if (shutdownTicks <= 1) {
                  PORTB &= ~LED;
                }
	}
	return 0;
}

/* ------------------------------------------------------------------------- */
