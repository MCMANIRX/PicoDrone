
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "screen.h"
#include "waveshare_frame.h"
#include "t_image.h"
#include "gfx3d.h"
#include "frame_test.h"
#include "barometer.h"
#include "run_motors.h"

float f_t;
float f_p;
           


uint16_t to565(uint32_t color) {

    return 				cc85[ color >> 16 & 0xff ] << 11
			    		| cc86[ color >>  8 & 0xff ] <<  5
			    		| cc85[ color       & 0xff ];
}

void bg(uint32_t color) {

    uint16_t c = to565(color);

    for(int i = 0; i < W*H; ++i)  
        fBuf[i] = c;
    
}


void test() {

    uint16_t c=0; 

    for(int i = 0; i < W*H; ++i)  
        fBuf[i] = c+i;
    
}

void draw_rect(uint8_t w, uint16_t h, uint8_t y, uint16_t x, uint32_t color) {

    uint16_t c  = to565(color);

    for(int i = 0 ; i < h; ++i)
        for(int j = 0 ; j < w; ++j)
            fBuf[(i+y)*W + x + j ] =c;
}


void img_put(uint w, uint h, uint x, uint y, uint32_t * image) {


    for(int i = 0 ; i < h; ++i)
        for(int j = 0 ; j < w; ++j)
            fBuf[(i+y)*W + j + x ] = (to565(image[(i*w)+j]) == 0xf81f) ?  fBuf[(i+y)*W + j + x ] : to565(image[(i*w)+j]);


}


void img_temp() {
    char temp_s[20];
    sprintf(temp_s,"%.5f deg. F",f_t);
    img_text(temp_s);

}

void img_pres() {
    char pres_s[20];
    sprintf(pres_s,"%.5f HPa",f_p);
    img_text(pres_s);

}


int main() {

    f_p,f_t = 0;

///////////////////////////////////////init////////////////////
    gpio_init(LED_PIN);
    gpio_init(RESET_PIN);

    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_set_dir(RESET_PIN, GPIO_OUT);

    
    gpio_put(LED_PIN,false);
    run_motors();
    gpio_put(LED_PIN,true);

        stdio_init_all();
  
  


   
   // bg(0xff0000);




     sleep_ms(500);


  //img_put(32,32,144,104,test_img2);


     // draw_rect(40,40,35,35,0x738adb);

 // img_put(320,240,0,0,test_img);

    // render3D(1,fBuf);


  //img_3d();

    //test();

    //init uart


 // sleep_ms(500);



 
    putchar_raw('S');

      img_init(fBuf);
  
  img_put(320,240,0,0,test_img);


    barometer_init();

        f_p = read_pres();

    f_t = read_temp('f');



        sleep_ms(500);

        for(int i = 0; i < 30; ++i){
        putchar_raw('s');
        sleep_ms(10);
}

while(1)    {


  //    img_put(320,240,0,0,test_img);
      gpio_put(LED_PIN, true);
  img_put(320,240,0,0,test_img);


    render3D(1,fBuf);

    img_temp();
    img_pres();
   



  puts_raw("one");
        
  for(int i = 0; i < 320*240; ++i){ 

   // printf("p%x-test",i);
      //  sleep_ms(1000);
         printf("%x",fBuf[i]);
          puts_raw("");
        }

  for(int k = 0; k < 50; ++k)
          puts_raw("end");

              gpio_put(LED_PIN,false);
              reset_y();


}



}