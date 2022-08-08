from machine import Pin, I2C, SoftI2C, SPI
import utime
#from OV5642_regs import *

pin = 0
i2d = 0
address = 0
b_size = 128


def spi_w_reg(addr,value):
    mask = 0x80
    buffer = bytearray(2)
    buffer[0] = addr | mask
    buffer[1] = value
    cs(0)
    spi.write(buffer)
    cs(1)

def spi_r_reg(addr):
    mask = 0x7f
    buffer = bytearray(1)
    buffer[0] = addr & mask
    cs(0)
    spi.write(buffer)
    spi.readinto(buffer)
    cs(1)
    return buffer

def i2c_w_reg(addr,value):
    buf = bytearray(3)
    buf[0] = (addr>>8) &0xff
    buf[1] = addr & 0xff
    buf[2] = value
    i2c.writeto(address,buf)
   # utime.sleep_ms(10)

def i2c_r_reg(addr):
    b = bytearray(2)
    rt = bytearray(1)
    b[0] = (addr>>8) &0xff
    b[1] = addr & 0xff
    i2c.writeto(address,b)
    utime.sleep_ms(1)
    rt = i2c.readfrom(address,1)
    return rt[0]

def i2c_w_regs(reg_val):
    for data in reg_val:
        addr = data[0]
        val = data[1]
        
        if(addr == 0xffff and val ==0xff):
            return
        i2c_w_reg(addr,val)
        utime.sleep(0.003)
        
def s():
    print(spi.read(0x04)[0])


def camCheck():
    #i2c_w_reg(0xff,0x01)
    id_h = i2c_r_reg(0x300a)
    id_l = i2c_r_reg(0x300b)

    if(id_h == 0x56 and id_l == 0x42):
        print("I2C functioning.")
        print("5MP Camera found (VB detected)")
    else:
        print("Could not ID camera. Exiting.")
        exit()
    return(id_h==0x56)

def camInit():
    print("Begin camInit...")
    utime.sleep_ms(100)

    i2c_w_reg(0x3008,0x80)

    i2c_w_regs(OV5642_QVGA_Preview1)
    i2c_w_regs(OV5642_QVGA_Preview1)
    utime.sleep(0.1)

    i2c_w_regs(OV5642_JPEG_Capture_QSXGA)
    i2c_w_regs(ov5642_320x240)
    utime.sleep(0.1)
    
    i2c_w_reg(0x3818,0xa8)
    i2c_w_reg(0x3621,0x10)
    i2c_w_reg(0x3801,0xb0)
    
     #compression quality
    i2c_w_reg(0x4407,0x02) # 2 = hi, 4 = md, 8 = lo
    
    #set sensor vsync timing active high
    spi_w_reg(0x03,0x02)
    utime.sleep(0.1)
    
    print("Init complete. Begin capture.")

def read_fifo_length():
    b0 = spi_r_reg(0x42)[0]
    b1 = spi_r_reg(0x43)[0]
    b2 = spi_r_reg(0x44)[0] & 0x7f
    
    return ((b2 << 16) | (b1 << 8) | (b0)) & 0x07fffff
    
def read_fifo_burst():
    fifo_buf = bytearray(b_size)
    count = 0
    length = read_fifo_length()
    print("reading image of length %d bytes..." % length)
                
    #set burst
    cs(0)
    spi.write(bytearray([0x3c]))    
    
    
    #read pixel data in 128-byte blocks
    while True:
        spi.readinto(fifo_buf)
        print(fifo_buf)
        count+=b_size
        
        if count+b_size >length:
            count = length-count
            fifo_buf = bytearray(count)
            spi.readinto(fifo_buf)
            print(read_fifo_length())

            cs(1)
            
            #clear fifo flag and break
            spi_w_reg(0x04,0x1)
            spi_w_reg(0x04,0x1)
            break
       
            
    
        
def captureFrames(frames):
    
    #clear fifo flag
    spi_w_reg(0x04,0x01)
    
    #set frame cnt (n+1)
    spi_w_reg(0x01,frames)
    
    #flush fifo, again to clear flag
    spi_w_reg(0x04,0x01)
    spi_w_reg(0x04,0x01)
                       
    #start capture
    spi_w_reg(0x04,0x02)
    print("capturing image...")
    #poll fifo write sr
    while(spi_r_reg(0x41)[0] & 8 == 0):
       cs(1)
    print("image captured.")
     
    read_fifo_burst()

    
    
    
    



if pin ==0:
    for _i2d in range(0,2):
        for _pin in range(1,40):
            c=1
            try:
                i2c = I2C(_i2d,scl=Pin(_pin),sda=Pin(_pin-1),freq=100000)
                scan = (i2c.scan())
            except ValueError as e:
                c=-1
            if scan != [] and c==1:
                address = scan[0]
                print("camera found at 0x%d on I2C id %d and SCL and SDA pins %d and %d." % (scan[0],_i2d,_pin,_pin-1))
                i2d = _i2d
                pin = _pin
              
                break

i2c = I2C(i2d,scl=Pin(pin),sda=Pin(pin-1),freq=100000)


#i2c.readfrom_mem(60,0x40,1)


 
spi = SPI(0,
          baudrate = 4000000,
          polarity = 0,
          phase = 0,
          bits = 8,
          sck = Pin(2),
          mosi = Pin(3),
          miso = Pin(4))

cs = Pin(5,Pin.OUT)



#spi init
spi_w_reg(0x07,0x80)
utime.sleep(0.1)
spi_w_reg(0x07,0x00)
utime.sleep(0.1)

#dummy spi read
cs.value(0)
spi.read(1)
utime.sleep(0.1)
cs.value(1)



#spi test
spi_w_reg(0x00,0x56)
utime.sleep(0.1)

value = spi_r_reg(0x00)

if(value[0] ==0x56):
    print("SPI functioning.")







#camera test
camCheck()

#init cam
camInit()

#start capture and print pixels
captureFrames(0)





           