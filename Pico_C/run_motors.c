#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/pwm.h"


int run_motors() {


    gpio_set_function(16, GPIO_FUNC_PWM);
    gpio_set_function(17, GPIO_FUNC_PWM);
    gpio_set_function(18, GPIO_FUNC_PWM);
    gpio_set_function(19, GPIO_FUNC_PWM);

    uint slice1 = pwm_gpio_to_slice_num(16);
    uint slice2 = pwm_gpio_to_slice_num(18);

    pwm_set_enabled(slice1, true);
    pwm_set_enabled(slice2, true);





    for(int i = 0; i <5000; ++i){

    pwm_set_gpio_level(16, i);

    pwm_set_gpio_level(17, i);

    pwm_set_gpio_level(18, i);

    pwm_set_gpio_level(19, i);

    sleep_us(100);


    }

    return 0;
    
    }

/*int main() {
    /// \tag::setup_pwm[]

    // Tell GPIO 0 and 1 they are allocated to the PWM
    gpio_set_function(16, GPIO_FUNC_PWM);
    gpio_set_function(17, GPIO_FUNC_PWM);

    // Find out which PWM slice is connected to GPIO 0 (it's slice 0)

    // Set period of 4 cycles (0 to 3 inclusive)
   // pwm_set_wrap(slice_num, 3);
    //// Set channel A output high for one cycle before dropping
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 1);
    // Set initial B output high for three cycles before dropping
   // pwm_set_chan_level(slice_num, PWM_CHAN_B, 3);
   pwm_set_gpio_level(16,32764);
   pwm_set_gpio_level(17,32764);
   
    // Set the PWM running
pwm_set_enabled(slice_num, true);
    /// \end::setup_pwm[]

    // Note we could also use pwm_set_gpio_level(gpio, x) which looks up the
    // correct slice and channel for a given GPIO.
}*/
