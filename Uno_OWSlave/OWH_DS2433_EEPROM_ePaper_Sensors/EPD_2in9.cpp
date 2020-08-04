/*****************************************************************************
* | File      	:	EPD_2in9.cpp
* | Author      :   Waveshare team
* | Function    :
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2018-06-07
* | Info        :   Basic version
*
******************************************************************************/
#include <stdlib.h>
#include "EPD_2in9.h"
#include "Debug.h"
#include "GUI_Paint.h"

const unsigned char lut_full_update[] =
{
    0x02, 0x02, 0x01, 0x11, 0x12, 0x12, 0x22, 0x22, 
    0x66, 0x69, 0x69, 0x59, 0x58, 0x99, 0x99, 0x88, 
    0x00, 0x00, 0x00, 0x00, 0xF8, 0xB4, 0x13, 0x51, 
    0x35, 0x51, 0x51, 0x19, 0x01, 0x00
};

const unsigned char lut_partial_update[] =
{
    0x10, 0x18, 0x18, 0x08, 0x18, 0x18, 0x08, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x13, 0x14, 0x44, 0x12, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/******************************************************************************
function :	Software reset
parameter:
******************************************************************************/
void EPD2IN9::EPD_Reset(void)
{
    EPD_RST_1;
    Dev->Dev_Delay_ms(200);
    EPD_RST_0;
    Dev->Dev_Delay_ms(200);
    EPD_RST_1;
    Dev->Dev_Delay_ms(200);
}

/******************************************************************************
function :	send command
parameter:
    Reg : Command register
******************************************************************************/
void EPD2IN9::EPD_SendCommand(UBYTE Reg)
{
    EPD_DC_0;
    EPD_CS_0;
    SPI_Write_Byte(Reg);
    EPD_CS_1;
}

/******************************************************************************
function :	send data
parameter:
    Data : Write data
******************************************************************************/
void EPD2IN9::EPD_SendData(UBYTE Data)
{
    EPD_DC_1;
    EPD_CS_0;
    SPI_Write_Byte(Data);
    EPD_CS_1;
}

/******************************************************************************
function :	Wait until the busy_pin goes LOW
parameter:
******************************************************************************/
void EPD2IN9::EPD_WaitUntilIdle(void)
{
    while(EPD_BUSY_RD == HIGH) {      //LOW: idle, HIGH: busy
        Dev->Dev_Delay_ms(100);
    }
}

/******************************************************************************
function :	Setting the display window
parameter:
******************************************************************************/
void EPD2IN9::EPD_SetWindows(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend)
{
    EPD_SendCommand(SET_RAM_X_ADDRESS_START_END_POSITION);
    EPD_SendData((Xstart >> 3) & 0xFF);
    EPD_SendData((Xend >> 3) & 0xFF);

    EPD_SendCommand(SET_RAM_Y_ADDRESS_START_END_POSITION);
    EPD_SendData(Ystart & 0xFF);
    EPD_SendData((Ystart >> 8) & 0xFF);
    EPD_SendData(Yend & 0xFF);
    EPD_SendData((Yend >> 8) & 0xFF);
}

/******************************************************************************
function :	Set cursor
parameter:
******************************************************************************/
void EPD2IN9::EPD_SetCursor(UWORD Xstart, UWORD Ystart)
{
    EPD_SendCommand(SET_RAM_X_ADDRESS_COUNTER);
    EPD_SendData((Xstart >> 3) & 0xFF);

    EPD_SendCommand(SET_RAM_Y_ADDRESS_COUNTER);
    EPD_SendData(Ystart & 0xFF);
    EPD_SendData((Ystart >> 8) & 0xFF);
    
    EPD_WaitUntilIdle();
}

/******************************************************************************
function :	Turn On Display
parameter:
******************************************************************************/
void EPD2IN9::EPD_TurnOnDisplay(void)
{
    EPD_SendCommand(DISPLAY_UPDATE_CONTROL_2);
    EPD_SendData(0xC4);
    EPD_SendCommand(MASTER_ACTIVATION);
    EPD_SendCommand(TERMINATE_FRAME_READ_WRITE);

    EPD_WaitUntilIdle();
}

/******************************************************************************
function :	Initialize the e-Paper register
parameter:
******************************************************************************/
UBYTE EPD2IN9::EPD_Init(const unsigned char* lut)
{
    Dev->System_Init();

    EPD_Reset();

    EPD_SendCommand(DRIVER_OUTPUT_CONTROL);
    EPD_SendData((EPD_HEIGHT - 1) & 0xFF);
    EPD_SendData(((EPD_HEIGHT - 1) >> 8) & 0xFF);
    EPD_SendData(0x00);                     // GD = 0; SM = 0; TB = 0;
    EPD_SendCommand(BOOSTER_SOFT_START_CONTROL);
    EPD_SendData(0xD7);
    EPD_SendData(0xD6);
    EPD_SendData(0x9D);
    EPD_SendCommand(WRITE_VCOM_REGISTER);
    EPD_SendData(0xA8);                     // VCOM 7C
    EPD_SendCommand(SET_DUMMY_LINE_PERIOD);
    EPD_SendData(0x1A);                     // 4 dummy lines per gate
    EPD_SendCommand(SET_GATE_TIME);
    EPD_SendData(0x08);                     // 2us per line
    EPD_SendCommand(DATA_ENTRY_MODE_SETTING);
    EPD_SendData(0x03);                     // X increment; Y increment
    
    //set the look-up table register
    EPD_SendCommand(WRITE_LUT_REGISTER);
    for (UWORD i = 0; i < 30; i++) {
        EPD_SendData(lut[i]);
    }
    return 0;
}

/******************************************************************************
function :	Clear screen
parameter:
******************************************************************************/
void EPD2IN9::EPD_Clear(void) 
{
    UWORD Width, Height;
    Width = (EPD_WIDTH % 8 == 0)? (EPD_WIDTH / 8 ): (EPD_WIDTH / 8 + 1);
    Height = EPD_HEIGHT;
    
    EPD_SetWindows(0, 0, EPD_WIDTH, EPD_HEIGHT);
    for (UWORD j = 0; j < Height; j++) {
        EPD_SetCursor(0, j);
        EPD_SendCommand(WRITE_RAM);
        for (UWORD i = 0; i < Width; i++) {
            EPD_SendData(0XFF);
        }
    }
    EPD_TurnOnDisplay();
}

/******************************************************************************
function :	Sends the image buffer in RAM to e-Paper and displays
parameter:
******************************************************************************/
void EPD2IN9::EPD_Display(void)
{
    UWORD Width, Height;
    Width = (EPD_WIDTH % 8 == 0)? (EPD_WIDTH / 8 ): (EPD_WIDTH / 8 + 1);
    Height = EPD_HEIGHT;

    EPD_SetWindows(0, 0, EPD_WIDTH, EPD_HEIGHT);
    for (UWORD j = 0; j < Height; j++) {
        EPD_SetCursor(0, j);
        EPD_SendCommand(WRITE_RAM);
        for (UWORD i = 0; i < Width; i++) {
            EPD_SendData(spiram->SPIRAM_RD_Byte(i + j * Width));
        }
    }
    EPD_TurnOnDisplay();
}

/******************************************************************************
function :	Enter sleep mode
parameter:
******************************************************************************/
void EPD2IN9::EPD_Sleep(void) 
{
    EPD_SendCommand(DEEP_SLEEP_MODE);
    EPD_WaitUntilIdle();
}
