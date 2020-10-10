/*****************************************************************************
* | File      	:	EPD_2in9.h
* | Author      :   Waveshare team
* | Function    :
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2018-06-07
* | Info        :   Basic version
*
******************************************************************************/
#ifndef _EPD2IN9_H
#define _EPD2IN9_H

#include "DEV_Config.h"
#include "SPI_RAM.h"

// Display resolution
#define EPD_WIDTH       128
#define EPD_HEIGHT      296

// EPD2IN9 commands
#define DRIVER_OUTPUT_CONTROL                       0x01
#define BOOSTER_SOFT_START_CONTROL                  0x0C
#define GATE_SCAN_START_POSITION                    0x0F
#define DEEP_SLEEP_MODE                             0x10
#define DATA_ENTRY_MODE_SETTING                     0x11
#define SW_RESET                                    0x12
#define TEMPERATURE_SENSOR_CONTROL                  0x1A
#define MASTER_ACTIVATION                           0x20
#define DISPLAY_UPDATE_CONTROL_1                    0x21
#define DISPLAY_UPDATE_CONTROL_2                    0x22
#define WRITE_RAM                                   0x24
#define WRITE_VCOM_REGISTER                         0x2C
#define WRITE_LUT_REGISTER                          0x32
#define SET_DUMMY_LINE_PERIOD                       0x3A
#define SET_GATE_TIME                               0x3B
#define BORDER_WAVEFORM_CONTROL                     0x3C
#define SET_RAM_X_ADDRESS_START_END_POSITION        0x44
#define SET_RAM_Y_ADDRESS_START_END_POSITION        0x45
#define SET_RAM_X_ADDRESS_COUNTER                   0x4E
#define SET_RAM_Y_ADDRESS_COUNTER                   0x4F
#define TERMINATE_FRAME_READ_WRITE                  0xFF

extern DEV *Dev;
extern const unsigned char lut_full_update[];
extern const unsigned char lut_partial_update[];

class EPD2IN9
{
    void EPD_Reset(void);
    void EPD_SendCommand(UBYTE Reg);
    void EPD_SendData(UBYTE Data);
    void EPD_WaitUntilIdle(void);
    void EPD_SetWindows(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend);
    void EPD_SetCursor(UWORD Xstart, UWORD Ystart) ;
    void EPD_TurnOnDisplay(void);

public:
    UBYTE EPD_Init(const unsigned char* lut);
    void EPD_Clear(void);
    void EPD_Display(void);
    void EPD_Sleep(void);

};


#endif /* EPD2IN13_H */

/* END OF FILE */
