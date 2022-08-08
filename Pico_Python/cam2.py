from machine import SPI, I2C, Pin
import utime
from OV5642_regs import *

pin = 0
i2d = 0
device=0


if pin == 0 :
    for _i2d in range(0,2):
        for _pin in range(1,40):
            c=1
            try:
                i2c = I2C(_i2d,scl=Pin(_pin),sda=Pin(_pin-1),freq=100000)
                scan = (i2c.scan())
            except ValueError as e:
                c=-1
            if scan != [] and c==1:
                device = scan[0]
                
                print("camera found at 0x%d on I2C id %d and SCL and SDA pins %d and %d." % (scan[0],_i2d,_pin,_pin-1))
                i2d = _i2d
                pin = _pin
                  
                break
            
            
cs = Pin(5,Pin.OUT,value=1)

spi = SPI(0,
          baudrate = 4000000,
          polarity = 0,
          phase = 0,
          bits = 8,
          sck = Pin(2),
          mosi = Pin(3),
          miso = Pin(4))

def spi_r(addr):
    mask = 0x7f
    buffer = bytearray(1)
    buffer[0] = addr & mask
    cs(0)
    spi.write(buffer)
    spi.readinto(buffer)
    cs(1)
    return buffer[0]

print(spi_r(0x40))



def spi_w(addr,value):
    cs(0)
    spi.write(bytearray(addr | 0x80,value))
    cs(1)


i2c = I2C(i2d,scl=Pin(pin),sda=Pin(pin-1),freq=100000)

def i2c_r(addr):
    i2c.writeto(device,bytearray(0x78,(addr >> 8) &0xff, addr &0xff, 0x79))
    utime.sleep_us(20)
    return i2c.readfrom(device,1)

def i2c_w(addr,data):
    i2c.writeto(device,bytearray((addr >> 8) &0xff, addr &0xff, bytearray(data)))
    utime.sleep_us(20)

def i2c_ws(reg):
    for data in reg:
        addr = data[0]
        val = data[1]
        
        if(addr == 0xffff and val ==0xff):
            return
        i2c_w(addr,val)
        #utime.sleep(0.003)

def fifo_len():
    b0 = spi_r(0x42)
    b1 = spi_r(0x43)
    b2 = spi_r(0x44) & 0x7f
    
    return ((b2 << 16) | (b1 << 8) | (b0)) & 0x07fffff

#reset firmware

spi_w(0x07,0x80)
utime.sleep(0.1)
spi_w(0x07,0x00)
utime.sleep(0.1)

#init cam
i2c_w(0x3008, 0x80)
i2c_ws(OV5642_QVGA_Preview1)
i2c_ws(OV5642_QVGA_Preview2)


i2c_ws(OV5642_JPEG_Capture_QSXGA)
i2c_ws(ov5642_320x240)
    
i2c_w(0x3818,0xa8)
i2c_w(0x3621,0x10)
i2c_w(0x3801,0xb0)
i2c_w(0x4407,0x04)

spi_w(0x04,0x01)
spi_w(0x04,0x01)

spi_w(0x04,0x02)

while(spi_r(0x41) & 8 == 0):
    print(spi_r(0x41))
print("gum")
   
leng = fifo_len()
print(leng)

cnt = leng
i=0

spi.write(bytearray(0x3c))

cs(0)
while True:
    temp = spi.read(1,0x00)
    print(temp)







