/*****************************************************************************
* | File        :   SPI_RAM.cpp
* | Author      :   Waveshare Team
* | Function    :   23LC1024 DRIVER
* | Info        :
*----------------
* | This version:   V1.0
* | Date        :   2018-05-12
* | Info        :   Basic version
*
******************************************************************************/
#include "DEV_Config.h"
#include "SPI_RAM.h"

/******************************************************************************
function:	Set the SPIRAM control mode
parameter:
See definition in SPIRAM.h, support byte mode, page mode, has any length mode
    BYTE_MODE
    PAGE_MODE
    STREAM_MODE
******************************************************************************/
void SPIRAM::SPIRAM_Set_Mode(UBYTE mode)
{
    SPIRAM_CS_0;

    SPI_Write_Byte(CMD_WRSR);
    SPI_Write_Byte(mode);

    SPIRAM_CS_1;
}

/******************************************************************************
function:	Read one byte
parameter:
    Addr : An address
******************************************************************************/
UBYTE SPIRAM::SPIRAM_RD_Byte(UDOUBLE Addr)
{
    UBYTE RD_Byte;

    SPIRAM_CS_0;
    SPI_Write_Byte(CMD_READ);

    SPI_Write_Byte((UBYTE)(Addr >> 16));
    SPI_Write_Byte((UBYTE)(Addr >> 8));
    SPI_Write_Byte((UBYTE)Addr);

    RD_Byte = SPI_Read_Byte(0x00);
    SPIRAM_CS_1;

    return RD_Byte;
}

/******************************************************************************
function:	Write a char data at an address
parameter:
    Addr : address
    Data : Written data
******************************************************************************/
void SPIRAM::SPIRAM_WR_Byte(UDOUBLE Addr, const UBYTE Data)
{
    // Write Addr,data
    SPIRAM_CS_0;

    SPI_Write_Byte(CMD_WRITE);

    SPI_Write_Byte((UBYTE)(Addr >> 16));
    SPI_Write_Byte((UBYTE)(Addr >> 8));
    SPI_Write_Byte((UBYTE)Addr);

    SPI_Write_Byte(Data);
    SPIRAM_CS_1;
}

/******************************************************************************
function:	Starting from an address, reading a page of data
parameter:
    Addr : address
    pBuf : A pointer to cache a page of data
******************************************************************************/
void SPIRAM::SPIRAM_RD_Page(UDOUBLE Addr, UBYTE *pBuf)
{
    UWORD i;

    // Write Addr, read data
    SPIRAM_CS_0;
    SPI_Write_Byte(CMD_READ);

    SPI_Write_Byte((UBYTE)(Addr >> 16));
    SPI_Write_Byte((UBYTE)(Addr >> 8));
    SPI_Write_Byte((UBYTE)Addr);

    for (i = 0; i < 32; i++) {
        *pBuf = SPI_Read_Byte(0x00);
        pBuf++;
    }
    SPIRAM_CS_1;
}

/******************************************************************************
function:	Write a page of data starting from an address
parameter:
    Addr : address
    pBuf : A pointer to cache a page of data
******************************************************************************/
void SPIRAM::SPIRAM_WR_Page(UDOUBLE Addr, UBYTE *pBuf)
{
    UWORD i;

    // Write Addr, read data
    SPIRAM_CS_0;
    SPI_Write_Byte(CMD_WRITE);

    SPI_Write_Byte((UBYTE)(Addr >> 16));
    SPI_Write_Byte((UBYTE)(Addr >> 8));
    SPI_Write_Byte((UBYTE)Addr);

    for (i = 0; i < 32; i++) {
        SPI_Write_Byte(*pBuf);
        pBuf++;
    }
    SPIRAM_CS_1;
}

/******************************************************************************
function:	Read data of length Len from an address and store it in the buffer pointing to pBuf
parameter:
    Addr : address
    pBuf : A pointer to cache a page of data
    Len  : The length of data read
******************************************************************************/
void SPIRAM::SPIRAM_RD_Stream(UDOUBLE Addr, UBYTE *pBuf, UDOUBLE Len)
{
    UWORD i;

    // Write Addr, read data
    SPIRAM_CS_0;
    SPI_Write_Byte(CMD_READ);

    SPI_Write_Byte((UBYTE)(Addr >> 16));
    SPI_Write_Byte((UBYTE)(Addr >> 8));
    SPI_Write_Byte((UBYTE)Addr);

    for (i = 0; i < Len; i++) {
        *pBuf = SPI_Read_Byte(0x00);
        pBuf++;
    }
    SPIRAM_CS_1;
}

/******************************************************************************
function:	Writing fixed-length data from an address
parameter:
    Addr : address
    pBuf : A pointer to cache a page of data
    Len  : The length of data write
******************************************************************************/
void SPIRAM::SPIRAM_WR_Stream(UDOUBLE Addr, UBYTE *pBuf, UDOUBLE Len)
{
    UWORD i;

    // Write Addr, read data
    SPIRAM_CS_0;
    SPI_Write_Byte(CMD_WRITE);

    SPI_Write_Byte((UBYTE)(Addr >> 16));
    SPI_Write_Byte((UBYTE)(Addr >> 8));
    SPI_Write_Byte((UBYTE)Addr);

    for (i = 0; i < Len; i++) {
        SPI_Write_Byte(*pBuf);
        pBuf++;
    }
    SPIRAM_CS_1;
}
