from machine import Pin, PWM
import time


for x in range(16,22 ):
    gp = Pin(x,Pin.OUT)


    pwm = PWM(gp)


    pwm.freq(25000)

    pwm.duty_u16(0)

    i=1000


    while(i<13000):
       pwm.duty_u16(i)
        
       i += 10
       time.sleep(0.01)
       print(i)
  