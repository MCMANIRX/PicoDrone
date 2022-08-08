#include "pico/stdlib.h"
#include "pico/binary_info.h"


int main() {


while(1)
for(int i = 1; i < 40  ; ++i){
    gpio_init(i);
    gpio_set_dir(i, GPIO_OUT);
    gpio_put(i,true);
    i++;
}

}