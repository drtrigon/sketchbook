/*****************************************************************************
* | File        :   EPD_SDCard.cpp
* | Author      :   Waveshare team
* | Function    :   Read the BMP file in the SD card and parse it
* | Info        :
*----------------
* | This version:   V1.0
* | Date        :   2018-01-11
* | Info        :   Basic version
*
******************************************************************************/
#include "EPD_SDCard.h"
#include "Debug.h"

BMP_HEADER BMP_Header;

/******************************************************************************
function:	Read 2 bytes of data and convert to big endian
parameter:
Info:
    These read data from the SD card file and convert them to big endian 
    (the data is stored in little endian format!)
******************************************************************************/
UWORD EPD_SDCARD::SDCard_Read16(File f)
{
    UWORD d;
    UBYTE b;
    b = f.read();
    d = f.read();
    d <<= 8;
    d |= b;
    return d;
}

/******************************************************************************
function:	Read 4 bytes of data and convert to big endian
parameter:
Info:
    These read data from the SD card file and convert them to big endian 
    (the data is stored in little endian format!)
******************************************************************************/
UDOUBLE EPD_SDCARD::SDCard_Read32(File f)
{
    UDOUBLE d;
    UWORD b;

    b = SDCard_Read16(f);
    d = SDCard_Read16(f);
    d <<= 16;
    d |= b;
    return d;
}

/******************************************************************************
function:	Reading BMP Picture Files header and Information header 
parameter:
Info:
    These read data from the SD card file and convert them to big endian 
    (the data is stored in little endian format!)
******************************************************************************/
boolean EPD_SDCARD::SDCard_ReadBmpHeader(File f)
{
    UWORD File_Type;
    File_Type = SDCard_Read16(f) ;//0000h 2byte: file type

    if (File_Type != 0x4D42) {
        // magic bytes missing
        return false;
    }

    // read file size
    BMP_Header.Size = SDCard_Read32(f);//0002h 4byte: file size

    // read and ignore creator bytes
    SDCard_Read32(f);//0006h 4byte:

    BMP_Header.Index = SDCard_Read32(f);//000ah 4byte: Offset between file head and image (offbits)

    // read DIB header
    BMP_Header.Header_Size = SDCard_Read32(f);//000Eh 4byte: Bitmap header = 40

    BMP_Header.BMP_Width = SDCard_Read32(f);//12-15:Image wide
    DEBUG("BMP_Width = ");
    DEBUG(BMP_Header.BMP_Width);
    DEBUG("\n");
    BMP_Header.BMP_Height = SDCard_Read32(f);//16-19:Image high
    DEBUG("BMP_Header = ");
    DEBUG(BMP_Header.BMP_Height);
    DEBUG("\n");
    if (SDCard_Read16(f) != 1)//1A-1B :0 for the use of the palette, 1 for the number of color indices
        return false;

    BMP_Header.Bit_Pixel = SDCard_Read16(f);//1C-1D：Whether the image is compressed

    if (SDCard_Read32(f) != 0) {//1E-21：
        // compression not supported!
        return false;
    }

    return true;
}

/******************************************************************************
function:	Initialize the SD card
parameter:
Info:
    If the initialization fails, stop the game until the 
    initialization is successful
******************************************************************************/
void EPD_SDCARD::SDCard_Init(void)
{
    SD_CS_1;

    Sd2Card card;
    card.init(SPI_FULL_SPEED, SD_CS);
    if (!SD.begin(SD_CS))  {
        DEBUG("SD init failed!\n");
        while (1);                              // init fail, die here
    }
    DEBUG("SD init OK!\n");
}

/******************************************************************************
function:	Read BMP image data to SPIRAM
parameter:
    BmpName : BMP picture name,
    Xstart : x starting position
    Ystart: Y starting position
Info:
    If the initialization fails, stop the game until the initialization is successful
******************************************************************************/
void EPD_SDCARD::SDCard_ReadBMP(const char *BmpName, UWORD Xstart, UWORD Ystart)
{
    File bmpFile;
    bmpFile = SD.open(BmpName);
    if (!bmpFile) {
        DEBUG("not find : ");
        DEBUG(BmpName);
        DEBUG("\n");
        return;
    } else {
        DEBUG("open bmp file : ");
        DEBUG(BmpName);
        DEBUG("\n");
    }

    if (!SDCard_ReadBmpHeader(bmpFile)) {
        DEBUG("read bmp file error\n");
        return;
    }

    bmpFile.seek(BMP_Header.Index);

    UWORD X, Y;
    UWORD Image_Width_Byte = (BMP_Header.BMP_Width % 8 == 0)? (BMP_Header.BMP_Width / 8): (BMP_Header.BMP_Width / 8 + 1);
    UWORD Bmp_Width_Byte = (Image_Width_Byte % 4 == 0) ? Image_Width_Byte: ((Image_Width_Byte / 4 + 1) * 4);

    UBYTE ReadBuff[1] = {0};
    for(Y = Ystart; Y < BMP_Header.BMP_Height; Y++) {//Total display column
        for(X = Xstart / 8; X < Bmp_Width_Byte; X++) {//Show a line in the line
            bmpFile.read(ReadBuff, 1);
            if(X < Image_Width_Byte) { //bmp
                if(Paint_Image.Image_Color == IMAGE_COLOR_POSITIVE) {
                    spiram->SPIRAM_WR_Byte(X + (BMP_Header.BMP_Height - Y - 1) * Image_Width_Byte + Paint_Image.Image_Offset, ReadBuff[0]);
                } else {
                    spiram->SPIRAM_WR_Byte(X + (BMP_Header.BMP_Height - Y - 1) * Image_Width_Byte + Paint_Image.Image_Offset, ~ReadBuff[0]);
                }
            }
        }
    }

    bmpFile.close();
}
