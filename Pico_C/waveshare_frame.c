//#include "DEV_Config.h"
#include "waveshare_frame.h"
#include "coords.h"


uint16_t bgr(uint16_t color) {

    return (color&0x1f) << 11 | (color>>5)&0x3f <<5  |( (color >> 11) &0x1f);
}


void img_init(uint16_t *img) {

    x=0;
    y=0;



    Paint_NewImage((uint8_t *)img,320,240,0,0x900a);

//set LCD color type to RGB565 (65K colors)
    Paint_SetScale(65); 
    
//set entire image to input color
 /*   Paint_Clear(0x900a);

*/
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


void img_3d() {

    Paint_Render3D(0);

    
}