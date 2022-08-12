//for bmp280
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "camera.h"
#include "barometer.h"

float t_fine = 0;

float tempCal[3];
float presCal[9];

float temperature; 

void cs(int on) {

    gpio_put(CS, on == 1 ? true : false);

}

void spi_w(uint8_t addr, uint8_t value) {

    uint8_t buf[2];
    buf[0] = addr & 0x7f;
    buf[1] = value;
    cs(0);
    spi_write_blocking(spi0,buf,2);
    cs(1);
    sleep_ms(1);

}

int spi_r(uint8_t addr, uint8_t* buf ,uint8_t len) {

    addr = (addr|0x80) &0xFF;
    cs(0);
    spi_write_blocking(spi0, &addr, 1);
    sleep_ms(100);

    spi_read_blocking(spi0,0,buf,len);
    cs(1);
    
    return buf[0];
    
}

void reset() {
    spi_w(0xE0, 0xb6);
}

void normal_mode() {
    spi_w(0xf4,0x3 | (0x2 <<5)); //normal mode
    spi_w(0xf5,0x5<<5); //50ms. meas. standby
}

void forced_mode() {
    spi_w(0xf4,0x2 | (0x2 <<5)); //forced
    spi_w(0xf5,0x5<<5); //50ms. meas. standby
}

void getCals() {

    uint8_t addr = 0x88;
    
   uint8_t* temp = (uint8_t*)malloc(2);

    for(int i = 0 ; i < 3; ++i) {
        spi_r(addr,temp,2);
        tempCal[i] = ((temp[1] << 8) | (temp[0] &0xff));
        addr+=0x2;
   //     printf("%f\n",tempCal[i]);
    }

    for(int i = 0 ; i < 9; ++i) {
        spi_r(addr,temp,2);
        presCal[i] = ((temp[1] << 8) | (temp[0] &0xff));
        addr+=0x2;
       // printf("%f\n",presCal[i]);
    }

    free(temp);
}

float read_temp(char temp) {

    uint8_t flag;

    uint8_t t_b[3]= {0};

    uint8_t addr = 0xfa;

    forced_mode();

    while(flag&0x08 == 0){
        spi_r(0xf3,&flag,1);
      //  printf("Spinning...\n");
    }

    free(flag);

    spi_r(addr,t_b,3);

   //    printf("%x %x %x\n",t_b[0],t_b[1],t_b[2]);


   float raw_temp = (float)((( (t_b[0] << 16) |(t_b[1] << 8) &0xff | (t_b[2] & 0xFF)) &0xFFFFFF)/16.0);

   float temp1 = (raw_temp/16384.0 - tempCal[0] / 1024.0) *tempCal[1] ; 

   float temp2 = (raw_temp/131072.0 - tempCal[0] / 8192.0) * (raw_temp / 131072.0 - tempCal[0] / 8192.0) * tempCal[2];


   normal_mode();


   float ret;

   ret = (float)((temp1+temp2)/5120.0);

   t_fine = ret;
   

   ret = (temp != 'f') ?  ret : ret*1.8 +32;

   //free(t_b);


   return ret;

}

float read_pres() {



    uint8_t p_b[3]= {0};

    spi_r((uint8_t)0xf7,p_b,3);


    float raw_pres = (float)((((p_b[0] << 16) | ((p_b[1] & 0xFF) << 8) | (p_b[2] & 0xFF)) & 0xFFFFFF)/16.0);

    float v1 = (float)(t_fine/2.0);
     v1 = v1 - (64.0 * 1000.0);

    float v2 = v1 * v1 * presCal[5]/32768.0;
    v2 = v2 + v1 * presCal[4] * 2.0;
    v2 = v2 / 4.0 + presCal[3] * 65536.0;

    float v3 = presCal[2] * v1 * v1 / 524288.0;

    v1 = (v3 + presCal[1] * v1) / 524288.0;

    v1 = (1.0 + v1 / 32768.0) * presCal[0];

    float pressure = 1048576.0 - raw_pres;
    pressure = ((pressure - v2 / 4096.0)*6250.0)/v1;

    v1 = presCal[8] * pressure * pressure / 2147483648.0f;

    v2 = pressure * presCal[7] / 32768.0f;

    pressure = pressure + (v1 + v2 + presCal[6]) /16.0f;

    pressure /= 100.0f;

    normal_mode();

    return pressure;

}

void main() {  

 // void  main() {

    stdio_init_all();

    spi_init(spi0,4*100*1000);
    gpio_set_function(SCK,GPIO_FUNC_SPI);
    gpio_set_function(MOSI,GPIO_FUNC_SPI);
    gpio_set_function(MISO,GPIO_FUNC_SPI);

    gpio_init(CS);
    gpio_set_dir(CS, GPIO_OUT);
    

 /* for(int i =0 ; i < 10; ++i) {
     printf("%d\n",i);
        sleep_ms(1000);
    }*/

    cs(1);
    reset();
    normal_mode();

    cs(0);
    spi_read_blocking(spi0,0,0,1);
    cs(1);
    getCals();


   /*1 while(1){

       printf("%f degrees, pressure is %f\n",read_temp('f'),read_pres());
       sleep_ms(1000);

    }*/

}