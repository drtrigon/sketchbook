#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include "gamepad.h"
#include "n64.h"

#include <usbdrv.h>

/******** IO port definitions **************/
#define N64_DATA_PORT	PORTB
#define N64_DATA_DDR	DDRB
#define N64_DATA_PIN	PINB
#define N64_DATA_BIT_A	(1<<3) // This also has to be changed in all asm routines
#define N64_DATA_BIT_B	(1<<4) // dito
#define N64_CAL 		95
#define GCN64_REPORT_SIZE 5

/*********** prototypes *************/
static void n64Init(void);
static void n64Update(void);
static char n64Changed(void);
static void n64BuildReport(unsigned char *reportBuffer);

/* What was most recently read from the controller */
static unsigned char report[GCN64_REPORT_SIZE];

/* What was most recently sent to the host */
static unsigned char reportSent[GCN64_REPORT_SIZE];
static unsigned char reportSent2[GCN64_REPORT_SIZE];

/* For probe */
static char last_failed_a;
static char last_failed_b;
static char pollTarget;

////////////////////////////////////////////////////////////////////////

static void n64Init(void)
{
	unsigned char sreg;
	sreg = SREG;
	cli();

	// data as input
	N64_DATA_DDR &= ~(N64_DATA_BIT_A | N64_DATA_BIT_B);

	// keep data low. By toggling the direction, we make the
	// pin act as an open-drain output.
	N64_DATA_PORT &= ~(N64_DATA_BIT_A | N64_DATA_BIT_B);

	SREG = sreg;
}

////////////////////////////////////////////////////////////////////////

void _tmpdata2report(unsigned volatile char results[65], uint8_t reportID) {
	int i;
	unsigned char tmpdata[4];	
  /*
	Bit	Function
	0	A
	1	B
	2	Z
	3	Start
	4	Directional Up
	5	Directional Down
	6	Directional Left
	7	Directional Right
	8	unknown (always 0)
	9	unknown (always 0)
	10	L
	11	R
	12	C Up
	13	C Down
	14	C Left
	15	C Right
	16-23: analog X axis
	24-31: analog Y axis
 */

	tmpdata[0]=0;
	tmpdata[1]=0;
	tmpdata[2]=0;
	tmpdata[3]=0;

	for (i=0; i<4; i++) // A B Z START
		tmpdata[2] |= results[i] ? (0x01<<i) : 0;

	for (i=0; i<4; i++) // C-UP C-DOWN C-LEFT C-RIGHT
		tmpdata[2] |= results[i+12] ? (0x10<<i) : 0;
	
	for (i=0; i<2; i++) // L R
		tmpdata[3] |= results[i+10] ? (0x01<<i) : 0;

	for (i=0; i<8; i++) // X axis
		tmpdata[0] |= results[i+16] ? (0x80>>i) : 0;
	
	for (i=0; i<8; i++) // Y axis
		tmpdata[1] |= results[i+24] ? (0x80>>i) : 0;	
	//Build our report
	report[0] = reportID; //Report ID

	// analog joystick
	report[1] = ((int)(((signed char)tmpdata[0]))*127/N64_CAL)+127;
	report[2] = ((int)((-((signed char)tmpdata[1])))*127/N64_CAL)+127;

	// buttons
	report[3] = tmpdata[2];
	report[4] = tmpdata[3];

	// dpad as buttons
	if (results[4]) 
		report[4] |= 0x04;
	if (results[5])
		report[4] |= 0x08;
	if (results[6])
		report[4] |= 0x10;
	if (results[7])
		report[4] |= 0x20;
}

void _n64UpdateA(unsigned char tmp)
{
	unsigned char volatile results[65];
	unsigned char count;
	
	/*
	 * z: Points to current source byte
	 * r16: Holds the bit to be AND'ed with current byte
	 * r17: Holds the current byte
	 * r18: Holds the number of bits left to send.
	 * r19: Loop counter for delays
	 *
	 * Edit sbi/cbi/andi instructions to use the right bit!
	 * 
         * original clock:               our clock:
	 * 3 us == 36 cycles             3 us = 49.5 cycles
	 * 1 us == 12 cycles             1 us = 16.5 cycles
	 */
	asm volatile(
"			push r30				\n"
"			push r31				\n"
			
"			ldi r16, 0x80			\n" // 1
"nextBit:							\n" 
"			mov r17, %2				\n" // 2
"			and r17, r16			\n" // 1
"			breq send0				\n" // 2
"			nop						\n"
// 1us low, 3us high (16.5, 36 cycles)
"send1:								\n"
"			sbi %1, 3				\n" // 2
"				ldi r19, 4			\n"	// 1
"lpxx0:			dec r19				\n"	// 1
"				brne lpxx0			\n"	// 2
"				nop\n			\n" // 3
"			cbi %1, 3				\n" // 2
"				ldi r19, 13			\n"	// 1 
"lp1:			dec r19				\n"	// 1 
"				brne lp1			\n"	// 2
"				\nnop		\n" // 2
"			lsr r16					\n" // 1
"			breq done				\n" // 1
"			rjmp nextBit			\n" // 2
		
/* Send a 0: 3us Low, 1us High */
"send0:		sbi %1, 3				\n"	// 2
"				ldi r19, 16			\n"	// 1
"lp0:			dec r19				\n"	// 1
"				brne lp0			\n"	// 2

"          	cbi %1,3				\n" // 2
"				nop\nnop\n				\n" // 2
"				nop\nnop\nnop\nnop	\n" // 4

"			lsr r16					\n" // 1
"			breq done				\n" // 1
"			rjmp nextBit			\n" // 2

"done:								\n"
"			cbi %1, 3				\n"
"				ldi r19, 4			\n"	// 1
"lpyy0:			dec r19				\n"	// 1
"				brne lpyy0			\n"	// 2


// Stop bit (1us low, 3us high)
"          	sbi %1,3				\n" // 2
"			nop\nnop\nnop\nnop		\n" // 4
"			nop\nnop\nnop\nnop		\n" // 4
"			cbi %1,3				\n" 
			

			// Response reading part //
			// r16: loop Counter
			// r17: Reference state
			// r18: Current state
			// r19: bit counter
		
"			ldi r19, 32\n			" // 1  We will receive 32 bits

"st:\n								"
"			ldi r16, 0xff			\n" // setup a timeout
"waitFall:							\n" 
"			dec r16					\n" // 1
"			breq timeout			\n" // 1 
"			in r17, %4				\n" // 1  read the input port
"			andi r17, 0x08			\n" // 1  isolate the input bit
"			brne waitFall			\n" // 2  if still high, loop

// OK, so there is now a 0 on the wire.
// Worst case, we are at the 9th cycle.
// Best case, we are at the 4th cycle.
// 	Middle: cycle 6
// 
// cycle: 1-12 12-24 24-36 36-48
//  high:  0     1     1     1
//	 low:  0     0     0     1
//
// I check the pin on the 24th (now: 33) cycle which is the safest place.

//"			cbi %5, 0\n"				// DEBUG
"			nop\nnop\n"

"				ldi r16, 7			\n"	// 1
"lpzz0:			dec r16				\n"	// 1
"				brne lpzz0			\n"	// 2

"			nop\n"
			
			// We are now more or less aligned on the 24th cycle.			
"			in r18, %4\n			" // 1  Read from the port

//"			sbi %5, 0\n"				// DEBUG
"			nop\nnop\n"

"			andi r18, 0x08\n		" // 1  Isolate our bit
"			st z+, r18\n			" // 2  store the value

"			dec r19\n				" // 1 decrement the bit counter
"			breq ok\n				" // 1

"			ldi r16, 0xff\n			" // setup a timeout
"waitHigh:\n						"
"			dec r16\n				" // decrement timeout
"			breq timeout\n			" // handle timeout condition
"			in r18, %4\n			" // Read the port
"			andi r18, 0x08\n		" // Isolate our bit
"			brne st\n				" // Continue if set
"			rjmp waitHigh\n			" // loop otherwise..

"ok:\n"
"			clr %0\n				"
"			rjmp end\n				"

"error:\n"
//"			sbi %5, 0\n"
"			nop\nnop\n"
"			mov %0, r19				\n" // report how many bits were read
"			rjmp end				\n"

"timeout:							\n"
"			clr %0					\n"
"			com %0					\n" // 255 is timeout

"end:\n"
"			pop r31\n"
"			pop r30\n"
//"			cbi %5, 0\n"
			: "=r" (count)
			: "I" (_SFR_IO_ADDR(N64_DATA_DDR)), "r"(tmp), 
				"z"(results), "I" (_SFR_IO_ADDR(N64_DATA_PIN)),
				"I" (_SFR_IO_ADDR(PORTB))
			: "r16","r17","r18","r19"
			);

	if (count == 255) {
		last_failed_a = 1;
		return; // failure
	} else last_failed_a = 0;
	
        _tmpdata2report(results, 0x02);
}



////////////////////////////////////////////////////////////////////////
void _n64UpdateB(unsigned char tmp)
{
	unsigned char volatile results[65];
	unsigned char count;

        // TODO(mlinder): This code can also be optimized to use
        //                more loops and less nops, thus saving
        //                up to 40-80 more bytes. Compare to _n64UpdateA
	
	/*
	 * z: Points to current source byte
	 * r16: Holds the bit to be AND'ed with current byte
	 * r17: Holds the current byte
	 * r18: Holds the number of bits left to send.
	 * r19: Loop counter for delays
	 *
	 * Edit sbi/cbi/andi instructions to use the right bit!
	 * 
	 * 3 us == 36 cycles
	 * 1 us == 12 cycles
	 */
	asm volatile(
"			push r30				\n"
"			push r31				\n"
			
"			ldi r16, 0x80			\n" // 1
"CnextBit:							\n" 
"			mov r17, %2				\n" // 2
//"			ldi r17, 0x0f			\n" // 2
"			and r17, r16			\n" // 1
"			breq Csend0				\n" // 2
"			nop						\n"

"Csend1:								\n"
"			sbi %1, 4				\n" // 2
"				nop\nnop\nnop\nnop	\n" // 4
"				nop\nnop\nnop\nnop	\n" // 4
"				nop\nnop\nnop\nnop	\n" // 4
"				nop\nnop\n			\n" // 3
"			cbi %1, 4				\n" // 2
"				ldi r19, 13			\n"	// 1 
"Clp1:			dec r19				\n"	// 1 
"				brne Clp1			\n"	// 2
"				nop			\n" // 2
"			lsr r16					\n" // 1
"			breq Cdone				\n" // 1
"			rjmp CnextBit			\n" // 2
		
/* Send a 0: 3us Low, 1us High */
"Csend0:		sbi %1, 4				\n"	// 2
"				ldi r19, 16			\n"	// 1
"Clp0:			dec r19				\n"	// 1
"				brne Clp0			\n"	// 2

"          	cbi %1,4				\n" // 2
"				nop\nnop\n				\n" // 2
"				nop\nnop\nnop\nnop	\n" // 4

"			lsr r16					\n" // 1
"			breq Cdone				\n" // 1
"			rjmp CnextBit			\n" // 2

"Cdone:								\n"
"			cbi %1, 4				\n"
"			nop\nnop\nnop\nnop		\n"	// 4
"			nop\nnop\nnop\nnop		\n"	// 4
"			nop\nnop\nnop\nnop		\n"	// 4
"			nop						\n"	// 1


// Stop bit (1us low, 3us high)
"          	sbi %1,4				\n" // 2
"			nop\nnop\nnop\nnop		\n" // 4
"			nop\nnop\nnop\nnop		\n" // 4
"			cbi %1,4				\n" 
			

			// Response reading part //
			// r16: loop Counter
			// r17: Reference state
			// r18: Current state
			// r19: bit counter
		
"			ldi r19, 32\n			" // 1  We will receive 32 bits

"Cst:\n								"
"			ldi r16, 0xff			\n" // setup a timeout
"CwaitFall:							\n" 
"			dec r16					\n" // 1
"			breq Ctimeout			\n" // 1 
"			in r17, %4				\n" // 1  read the input port
"			andi r17, 0x10			\n" // 1  isolate the input bit
"			brne CwaitFall			\n" // 2  if still high, loop

// OK, so there is now a 0 on the wire.
// Worst case, we are at the 9th cycle.
// Best case, we are at the 4th cycle.
// 	Middle: cycle 6
// 
// cycle: 1-12 12-24 24-36 36-48
//  high:  0     1     1     1
//	 low:  0     0     0     1
//
// I check the pin on the 24th cycle which is the safest place.

//"			cbi %5, 0\n"				// DEBUG
"			nop\nnop\n"
"			nop\nnop\n	" // 2
"			nop\nnop\nnop\nnop\n	" // 4
"			nop\nnop\nnop\nnop\n	" // 4
"			nop\nnop\nnop\nnop\n	" // 4
"			nop\nnop\nnop\nnop\n	" // 4
"			nop\nnop\nnop\nnop\n	" // 4
"			nop\n"
			
			// We are now more or less aligned on the 24th cycle.			
"			in r18, %4\n			" // 1  Read from the port
//"			sbi %5, 0\n"				// DEBUG
"			nop\nnop\n"
"			andi r18, 0x10\n		" // 1  Isolate our bit
"			st z+, r18\n			" // 2  store the value

"			dec r19\n				" // 1 decrement the bit counter
"			breq Cok\n				" // 1

"			ldi r16, 0xff\n			" // setup a timeout
"CwaitHigh:\n						"
"			dec r16\n				" // decrement timeout
"			breq Ctimeout\n			" // handle timeout condition
"			in r18, %4\n			" // Read the port
"			andi r18, 0x10\n		" // Isolate our bit
"			brne Cst\n				" // Continue if set
"			rjmp CwaitHigh\n			" // loop otherwise..

"Cok:\n"
"			clr %0\n				"
"			rjmp Cend\n				"

"Cerror:\n"
//"			sbi %5, 0\n"
"			nop\nnop\n"
"			mov %0, r19				\n" // report how many bits were read
"			rjmp Cend				\n"

"Ctimeout:							\n"
"			clr %0					\n"
"			com %0					\n" // 255 is timeout

"Cend:\n"
"			pop r31\n"
"			pop r30\n"
//"			cbi %5, 0\n"
			: "=r" (count)
			: "I" (_SFR_IO_ADDR(N64_DATA_DDR)), "r"(tmp), 
				"z"(results), "I" (_SFR_IO_ADDR(N64_DATA_PIN)),
				"I" (_SFR_IO_ADDR(PORTB))
			: "r16","r17","r18","r19"
			);

	if (count == 255) {
		last_failed_b = 1;
		return; // failure
	}
        else last_failed_b = 0;
		
        _tmpdata2report(results, 0x01);
}
////////////////////////////////////////////////////////////////////////
static void n64Update(void)
{
	pollTarget++;

        while (1) {
          unsigned char sof = usbSofCount; 
 
          // Fetch controlle data
  	  if (pollTarget & 0b1) _n64UpdateA(0x01); // get status
	  else _n64UpdateB(0x01); // get status

          // Check if an usb interrupt occured
          if (sof == usbSofCount)
              break;
          else
              _delay_us(50); // interrupt occured while reading data = data unusable
        }
}
////////////////////////////////////////////////////////////////////////
static char n64Probe(void)
{
	char i;

	for (i=0; i<10; i++)
	{
		_delay_ms(20);
		
		last_failed_a = 1;
		last_failed_b = 1;

                n64Update();
                n64Update();

		if (!last_failed_a || !last_failed_b)
			return 1;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////
static char n64Changed(void)
{
	static int first = 1;
	if (first) { first = 0;  return 1; }
	
	return memcmp(report, reportSent2, GCN64_REPORT_SIZE);
}
////////////////////////////////////////////////////////////////////////
static void n64BuildReport(unsigned char *reportBuffer)
{
	if (reportBuffer) 
		memcpy(reportBuffer, report, GCN64_REPORT_SIZE);
	
	memcpy(reportSent2, reportSent, GCN64_REPORT_SIZE);	
	memcpy(reportSent, report, GCN64_REPORT_SIZE);	
}
////////////////////////////////////////////////////////////////////////
Gamepad N64Gamepad = {
	.report_size			= GCN64_REPORT_SIZE,
	.init					= n64Init,
	.update					= n64Update,
	.changed				= n64Changed,
	.buildReport			= n64BuildReport,
	.probe					= n64Probe,
};
////////////////////////////////////////////////////////////////////////
Gamepad *n64GetGamepad(void)
{
	return &N64Gamepad;
}

