#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"

#include "camera.h"
#include "ov5642_regs.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


uint8_t address = 0x3c;

uint8_t rx = 0;


void cs(int on) {

    gpio_put(CS, on == 1 ? true : false);
}


void spi_w(uint8_t addr, uint8_t value) {

    uint8_t buf[2];
    buf[0] = addr | 0x80;
    buf[1] = value;
    cs(0);
    spi_write_blocking(spi0,buf,2);
    cs(1);
    sleep_ms(1);

}

int spi_r(uint8_t addr) {

    uint8_t value = 0;
    addr = addr &0x7f;
    cs(0);
    spi_write_blocking(spi0, &addr, 1);
    spi_read_blocking(spi0,0,&value,1);
    cs(1);
    return value;
    
}

int i2c_w(uint16_t addr, uint8_t value) {

    uint8_t buf[3] = {0};
    buf[0] = addr >>8;
    buf[1] = addr &0xff;
    buf[2] = value;

    i2c_write_blocking(i2c0,address,buf,3,true);
    return 1;
}

void i2c_ws(const struct sensor_reg reglist[])
{
		int err = 0;
	  unsigned int reg_addr;
	  unsigned char reg_val;
	  const struct sensor_reg *next = reglist;
	  while ((reg_addr != 0xffff) | (reg_val != 0xff))
	  {
	     reg_addr = next->reg;
	     reg_val = next->val;
	    err = i2c_w(reg_addr, reg_val);
	    next++;
	  }
}

uint8_t i2c_r(uint16_t addr, uint8_t* ret) {

    uint8_t buf[2] = {0};
    buf[0] = (addr >> 8)&0xff;
    buf[1] = (addr & 0xff);

    i2c_write_blocking(i2c0, address, buf,2,true);
    i2c_read_blocking(i2c0,address,ret,1,false);
    return 1;


    
}


int camCheck() {
    uint8_t h;
    uint8_t l;
    i2c_r(0x300a,&h);
    i2c_r(0x300b,&l);

    if(h == 0x56 && l ==0x42)
        return 1;
    else return 0;
}


int camInit() {
    printf("Begin camInit....\n");

    i2c_w(0x3008,0x80);
/*
    i2c_ws(OV5642_QVGA_Preview);
    sleep_ms(100);

    i2c_ws(OV5642_JPEG_Capture_QSXGA);
    i2c_ws(ov5642_320x240);
    sleep_ms(100);

    i2c_w(0x3818,0xa8);
    i2c_w(0x3621,0x10);
    i2c_w(0x3801,0xb0);

    //JPEG compression quaity
    i2c_w(0x4407,0x02); // 2 = hi, 4 = med, 8 = lo

    */

    
    i2c_ws(OV5642_1280x960_RAW); //inits raw
    i2c_ws(OV5642_640x480_RAW);

    uint8_t reg_val;

    i2c_w(0x4740, 0x21);
	i2c_w(0x501e, 0x2a);
	i2c_w(0x5002, 0xf8);
	i2c_w(0x501f, 0x01);
	i2c_w(0x4300, 0x61);
	i2c_r(0x3818, &reg_val);
	i2c_w(0x3818, (reg_val | 0x60) & 0xff);
	i2c_r(0x3621, &reg_val);
	i2c_w(0x3621, reg_val & 0xdf);

    //set sensor vsync timing active high
    spi_w(0x03,0x02);
    sleep_ms(100);

    printf("Init complete. Begin capture.\n");
}

int read_fifo_len() {
    uint8_t b0 = spi_r(0x42);
    uint8_t b1 = spi_r(0x43);
    uint8_t b2 = spi_r(0x44);
    return ((b2 << 16) | (b1 << 8) | b0)  &0x07fffff;
}

uint8_t* read_fifo_burst() {

    uint8_t w = 0x3c;
    
    int len = read_fifo_len();

    printf("Size of image is %d bytes.\n",len);
    uint8_t *imgBuf = (uint8_t*) malloc(len*sizeof(uint8_t));
    cs(0);

    spi_write_blocking(spi0,&w,1);
    spi_read_blocking(spi0,&w,imgBuf,len);

    cs(1);
    
    int count = 0;

    while(count <= len)
        printf("%x",imgBuf[count++]);

// *stbi_load_from_memory   (stbi_uc           const *buffer, int len   , int *x, int *y, int *channels_in_file, int desired_channels);
    uint8_t* image = stbi_load_from_memory(imgBuf,len,320,240,3,3);
    return image;
}

void captureFrame(uint8_t pics) {

    //clear fifo flag
    spi_w(0x04,0x1);

    //set frame cnt (n+1)
    spi_w(0x01,pics);


    //flush fifo, again to clear flag
    spi_w(0x04,0x1);
    spi_w(0x04,0x1);

    //start capture
    spi_w(0x04,0x2);

    int k = 0;
    printf("Capturing image");

    while(spi_r(0x41) & 8 == 0)
        if(k%0x20 == 0)
            printf(".");

    printf("\nCapture complete.\n");

    return read_fifo_burst();


}


int main() {

    //blink LED to begin

    const uint led_pin = 25;

    gpio_init(led_pin);

    gpio_set_dir(led_pin, GPIO_OUT);
    
  //  while(1) {

    gpio_put(led_pin,false);
    gpio_put(led_pin, true);
    sleep_ms(300);
    gpio_put(led_pin,false);
 //   }



    //init uart
    stdio_init_all();


    //init i2c
    i2c_init(i2c0,100*1000);
    gpio_set_function(SDA,GPIO_FUNC_I2C);
    gpio_set_function(SCL,GPIO_FUNC_I2C);

    gpio_pull_up(SDA);
    gpio_pull_up(SCL);

    bi_decl(bi_2pins_with_func(SDA,SCL,GPIO_FUNC_I2C))

    //init spi
    spi_init(spi0,4*100*1000);
    gpio_set_function(SCK,GPIO_FUNC_SPI);
    gpio_set_function(MOSI,GPIO_FUNC_SPI);
    gpio_set_function(MISO,GPIO_FUNC_SPI);

    gpio_init(CS);
    gpio_set_dir(CS, GPIO_OUT);
    

   for(int i =0 ; i < 10; ++i) {
     printf("%d\n",i);
        sleep_ms(1000);
    }


//I2C and cam test

while(1){
    if(camCheck())
        break;
    else 
        printf("Cannot find camera.\n");
    sleep_ms(500);

}

printf("I2C Functioning.\n");
printf("5MP camera found (VB detected)\n");

//dummy spi read
cs(0);
spi_read_blocking(spi0,0,0,1);
cs(1);

//SPI test
spi_w(0x00,0x56);
rx = spi_r(0x00);

if(rx == 0x56)
    printf("SPI Functioning.\n");


//init cam
camInit();

captureFrame(0);

}