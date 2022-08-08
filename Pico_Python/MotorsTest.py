from machine import Pin, PWM
import time


def fadeLED(pin):
    pwm = PWM(pin)

    pwm.freq(1000)
    duty = 0
    direction = 1


    for _ in range(256):
        duty += direction
        if duty > 255:
            duty = 255
            direction = -1

        elif duty < 0:
            duty = 0
            direction = 1
        pwm.duty_u16(duty * duty)
        time.sleep(0.01)
        
gp0 = Pin(0,Pin.OUT)
gp1 = Pin(1,Pin.OUT)
gp2 = Pin(2,Pin.OUT)
gp3 = Pin(3,Pin.OUT)
        
while(True):
    fadeLED(gp0)
    fadeLED(gp1)
    fadeLED(gp2)
    fadeLED(gp3)








