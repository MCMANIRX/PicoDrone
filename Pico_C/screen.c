

//3 line 9 bit serial interface

//1 1 0 1

//9th bit command bit

//low, command byte 
//high, RAM write or command register param

//big endian

//cs high, serial init

//reset pin turns on screen


//for rgb565, RIM = 0, 0X3A, 0b101

//before ram write, window must be declared

//x1 = 0, y1 = 0, x2=239 (0xef), y2=319 (0x13f)


#include "screen.h"

void cs(int on) {

    gpio_put(CS, on == 1 ? true : false);
}


void w(uint8_t bit,int8_t addr, uint8_t *data, uint8_t len){
    
    uint8_t *buf = malloc((len+1)*2);

    buf[0] = 0;
    buf[1] = addr;

    uint8_t pos = 0;

    for(uint8_t i = 0 ; i < len*2; ++i) {
        buf[i+2] = 0x1;
        buf[i+3] = data[pos++]; 
        i++;      

        }

    cs(0);
    spi_write_blocking(spi0,buf,len);
    cs(1);
    sleep_ms(1);
    free(buf);
}

void cmd_w(uint8_t addr, uint8_t *data, uint8_t len) {

    w(0x0,addr, data, len);
}

void ram_w(uint8_t addr, uint8_t *data, uint8_t len) {

    w(0x1,addr, data, len);
}

uint8_t * r(uint8_t addr, uint8_t len) {

    uint8_t *ret= malloc(len);
    cs(0);
    spi_write_blocking(spi0, &addr, 1);
    cs(1);
    sleep_ms(1);
    cs(0);
    spi_read_blocking(spi0,0,&ret,len);
    cs(1);

    return ret;

}

void reset() {

    gpio_put()

}

void init() {
    
    reset();
    
    cmd_w(ST7789_SWRESET);	//1: Software reset, no args, w/delay: delay(150)
	sleep_ms(1);

	cmd_w(ST7789_SLPOUT);	// 2: Out of sleep mode, no args, w/delay: delay(500)
	sleep_ms(1);

	cmd_w(ST7789_COLMOD);	// 3: Set color mode, 1 arg, delay: delay(10)
	_st_write_data_8bit(ST7789_COLOR_MODE_65K | ST7789_COLOR_MODE_16BIT);	// 65K color, 16-bit color
	sleep_ms(1);

	cmd_w(ST7789_MADCTL);	// 4: Memory access ctrl (directions), 1 arg:
	_st_write_data_8bit(ST7789_MADCTL_RGB);	// RGB Color

	cmd_w(ST7789_INVON);	// 5: Inversion ON (but why?) delay(10)
	sleep_ms(1);

	cmd_w(ST7789_NORON);	// 6: Normal display on, no args, w/delay: delay(10)
	sleep_ms(1);

	cmd_w(ST7789_DISPON);	// 7: Main screen turn on, no args, w/delay: delay(500)
	sleep_ms(1);

}



int main() {

///////////////////////////////////////init////////////////////
    gpio_init(LED_PIN);
    gpio_init(RESET_PIN)

    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_set_dir(RESET_PIN, GPIO_OUT);

    
  //  while(1) {

    gpio_put(LED_PIN,false);
    gpio_put(LED_PIN, true);
    sleep_ms(300);
    gpio_put(LED_PIN,false);
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

    cs(1);
///////////////////////////////////////////////////////////////

    init();

}