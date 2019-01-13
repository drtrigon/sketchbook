// derived from "epd2in9-demo"
// see also:
//   https://www.bastelgarage.ch/universal-raw-driver-e-paper-shield-fur-arduino-nucleo?search=epaper
//   https://www.waveshare.com/wiki/E-Paper_Shield
//   https://www.waveshare.com/w/upload/c/c8/E-Paper_Shield_User_Manual_en.pdf
//   https://www.waveshare.com/w/upload/b/bb/E-Ppaer_Shield_Schematic.pdf
//   https://www.waveshare.com/w/upload/e-Paper_Shield_Code.7z
//   https://github.com/ZinggJM/GxEPD

#include "EPD_2in9.h"
#include "GUI_Paint.h"
//#include "EPD_SDCard.h"
#include "ImageData.h"
#include <Wire.h>

void setup()
{
    GUIPAINT paint;
    EPD2IN9 epd;
    //EPD_SDCARD SD;
    
    //Initialize e-Paper
    if (epd.EPD_Init(lut_full_update) != 0) {
        Serial.print("e-Paper init failed");
        return;
    }
    Serial.print("2.9inch e-Paper demo\n");

    //1.Create a new image cache named IMAGE_BW and fill it with white
    UBYTE Rotate = IMAGE_ROTATE_90;
//    UBYTE Rotate = IMAGE_ROTATE_180;
    paint.Paint_NewImage(IMAGE_BW, EPD_WIDTH, EPD_HEIGHT, Rotate, IMAGE_COLOR_POSITIVE);
    paint.Paint_Clear(WHITE);
    
    //2.Drawing on the image
    if (Rotate == IMAGE_ROTATE_0 || Rotate == IMAGE_ROTATE_180) {
        Serial.print("Vertical screen\n");
        
        paint.Paint_DrawPoint(10, 20, BLACK, DOT_PIXEL_1X1, DOT_STYLE_DFT);
        paint.Paint_DrawPoint(10, 30, BLACK, DOT_PIXEL_2X2, DOT_STYLE_DFT);
        paint.Paint_DrawPoint(10, 40, BLACK, DOT_PIXEL_3X3, DOT_STYLE_DFT);

        paint.Paint_DrawLine(20, 20, 70, 70, BLACK, LINE_STYLE_SOLID, DOT_PIXEL_1X1);
        paint.Paint_DrawLine(70, 20, 20, 70, BLACK, LINE_STYLE_SOLID, DOT_PIXEL_1X1);
        paint.Paint_DrawLine(30, 120, 60, 120, BLACK, LINE_STYLE_DOTTED, DOT_PIXEL_1X1);
        paint.Paint_DrawLine(45, 135, 45, 105, BLACK, LINE_STYLE_DOTTED, DOT_PIXEL_1X1);

        paint.Paint_DrawRectangle(20, 20, 70, 70, BLACK, DRAW_FILL_EMPTY, DOT_PIXEL_1X1);
        paint.Paint_DrawRectangle(75, 20, 120, 70, BLACK, DRAW_FILL_FULL, DOT_PIXEL_1X1);

        paint.Paint_DrawCircle(45, 120, 15, BLACK, DRAW_FILL_EMPTY, DOT_PIXEL_1X1);
        paint.Paint_DrawCircle(100, 120, 15, BLACK, DRAW_FILL_FULL, DOT_PIXEL_1X1);

        paint.Paint_DrawString_EN(10, 150, "hello world", &Font12, WHITE, BLACK);
        paint.Paint_DrawString_EN(10, 170, "waveshare", &Font16, BLACK, WHITE);

        paint.Paint_DrawNum(10, 200, 123456789, &Font12, BLACK, WHITE);
        paint.Paint_DrawNum(10, 220, 987654321, &Font16, WHITE, BLACK);
    } else {
        Serial.print("Horizontal screen\n");
            
        paint.Paint_DrawPoint(10, 80, BLACK, DOT_PIXEL_1X1, DOT_STYLE_DFT);
        paint.Paint_DrawPoint(10, 90, BLACK, DOT_PIXEL_2X2, DOT_STYLE_DFT);
        paint.Paint_DrawPoint(10, 100, BLACK, DOT_PIXEL_3X3, DOT_STYLE_DFT);

        paint.Paint_DrawLine(20, 70, 70, 120, BLACK, LINE_STYLE_SOLID, DOT_PIXEL_1X1);
        paint.Paint_DrawLine(70, 70, 20, 120, BLACK, LINE_STYLE_SOLID, DOT_PIXEL_1X1);
        paint.Paint_DrawLine(140, 95, 180, 95, BLACK, LINE_STYLE_DOTTED, DOT_PIXEL_1X1);
        paint.Paint_DrawLine(160, 75, 160, 115, BLACK, LINE_STYLE_DOTTED, DOT_PIXEL_1X1);

        paint.Paint_DrawRectangle(20, 70, 70, 120, BLACK, DRAW_FILL_EMPTY, DOT_PIXEL_1X1);
        paint.Paint_DrawRectangle(80, 70, 130, 120, BLACK, DRAW_FILL_FULL, DOT_PIXEL_1X1);

        paint.Paint_DrawCircle(160, 95, 20, BLACK, DRAW_FILL_EMPTY, DOT_PIXEL_1X1);
        paint.Paint_DrawCircle(210, 95, 20, BLACK, DRAW_FILL_FULL, DOT_PIXEL_1X1);

        paint.Paint_DrawString_EN(10, 0, "waveshare Electronics", &Font16, BLACK, WHITE);
        paint.Paint_DrawString_EN(10, 20, "hello world", &Font12, WHITE, BLACK);

        paint.Paint_DrawNum(10, 33, 123456789, &Font12, BLACK, WHITE);
        paint.Paint_DrawNum(10, 50, 987654321, &Font16, WHITE, BLACK);        
    }

    //3.Refresh the picture in RAM to e-Paper
    epd.EPD_Display();
//    epd.EPD_Sleep();      // prevent ghosting and other damages, e.g. due to high-voltage for long time
    Dev->Dev_Delay_ms(2000);//Analog clock 1s
    
    //4.Partial refresh, example shows time
    if (epd.EPD_Init(lut_partial_update) != 0) {
        Serial.print("e-Paper init failed");
        return;
    }

    PAINT_TIME sPaint_time;
    sPaint_time.Hour = 12;
    sPaint_time.Min = 34;
    sPaint_time.Sec = 56;
    for (;;) {
        sPaint_time.Sec = sPaint_time.Sec + 1;
        if (sPaint_time.Sec == 60) {
            sPaint_time.Min = sPaint_time.Min + 1;
            sPaint_time.Sec = 0;
            if (sPaint_time.Min == 60) {
                sPaint_time.Hour =  sPaint_time.Hour + 1;
                sPaint_time.Min = 0;
                if (sPaint_time.Hour == 24) {
                    sPaint_time.Hour = 0;
                    sPaint_time.Min = 0;
                    sPaint_time.Sec = 0;
                }
            }
        }
        if (Rotate == IMAGE_ROTATE_0 || Rotate == IMAGE_ROTATE_180) {
            paint.Paint_ClearWindows(15, 250, 15 + Font20.Width * 7, 250 + Font20.Height, WHITE);
            paint.Paint_DrawTime(15, 75, &sPaint_time, &Font20, WHITE, BLACK);
        } else {
            paint.Paint_ClearWindows(120, 30, 120 + Font20.Width * 7, 30 + Font20.Height, WHITE);
            paint.Paint_DrawTime(120, 30, &sPaint_time, &Font20, WHITE, BLACK);
        }

        epd.EPD_Display();
//        epd.EPD_Sleep();  // prevent ghosting and other damages, e.g. due to high-voltage for long time
        Dev->Dev_Delay_ms(500);//Analog clock 1s
    }
}

void loop()
{

}
