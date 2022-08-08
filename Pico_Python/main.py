from machine import Pin, PWM
import time


def blincLED(pin):
    pin.toggle()
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




#gp0 = Pin(0, Pin.OUT)
#gp1 = Pin(1, Pin.OUT)
#gp2 = Pin(2, Pin.OUT)

#gp0 = Pin(0, Pin.OUT)
#gp1 = Pin(1, Pin.OUT)
#gp2 = Pin(2, Pin.OUT)
blincLED(Pin(25,Pin.OUT))
while(True):
    for x in range(40):
        gp = Pin(x,Pin.OUT)
        fadeLED(gp)
        time.sleep(0.01)
        
        #GNDs at 3,8,13,18,23,28,33,38
        #last GPIO at 32
        #run at 30



#gp0.toggle()
#gp1.toggle()
#gp2.toggle()


