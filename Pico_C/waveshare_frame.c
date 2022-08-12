//#include "DEV_Config.h"
#include "waveshare_frame.h"
#include "coords.h"


#define HORIZONTAL 0
#define VERTICAL   1

uint16_t bgr(uint16_t color) {

    return (color&0x1f) << 11 | (color>>5)&0x3f <<5  |( (color >> 11) &0x1f);
}


void img_init(uint16_t *img) {

    x=0;
    y=0;

    DEV_Delay_ms(100);

     if(DEV_Module_Init()!=0){
        return -1;
    }
        DEV_SET_PWM(50);


    LCD_2IN_Init(HORIZONTAL);
    LCD_2IN_Clear(0xf800);



    Paint_NewImage((uint8_t *)img,320,240,90,0x900a);

//set LCD color type to RGB565 (65K colors)
    Paint_SetScale(65);
    Paint_Clear(0xffff);
    Paint_SetRotate(ROTATE_270);

    LCD_2IN_Display((UBYTE *)img);
    DEV_Delay_ms(1000);

}

void img_text(const char *txt) {
   uint len = sizeof(txt)/sizeof(char);

    //      x,  y,  str,    font,   bg, fg
    Paint_DrawString_EN(x,y, txt, &Font20, ALPHA, 0xf800);
    y+=20;

    //changed GUI_Paint.c colors to big from little endian (lines 187-188)

}

void reset_y(  ){
    y=0;
}


void img_3d(uint16_t *buff, uint32_t* test_img) {

   // Paint_Render3D(0);

    Paint_DrawImage1((UBYTE*)(test_img),0,0,320,240);
    LCD_2IN_Display((UBYTE *)buff);

    
}