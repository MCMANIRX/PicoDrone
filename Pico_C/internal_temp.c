#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "vga_config.h"

int main() {
    stdio_init_all();

    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4);

    uint16_t raw =  0;

    while(1) {

        raw = adc_read();
        printf("temp is %f degrees F.\n",(float)((27-(((raw*(3.3f/(1<<12)))-0.706)/0.001721))*1.8 +32));
        sleep_ms(500);
    }   
}