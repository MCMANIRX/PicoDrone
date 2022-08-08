from machine import SPI, Pin
import utime

tempCal = [0,0,0]
presCal = [0,0,0,0,0,0,0,0,0]
t_fine = 0

def r_reg(addr,leng):
    reg = (addr | 0x80) & 0xff
    cs(0)
    spi.write(bytearray([reg]))
    ret = bytearray(leng)
    spi.readinto(ret)
    cs(1)
    return ret


def w_reg(addr,data):
    reg = (addr & 0x7f) & 0xff
    cs(0)
    spi.write(bytearray([reg,data & 0xff]))
    cs(1)

def reset():
     w_reg(0xE0, 0xB6)
     
def normal():
    w_reg(0xF4,0x3 | (0x2 << 5)) # set barometer to normal mode
    w_reg(0xF5,0x5<<5) # 1 min. meas standby

def force():
    w_reg(0xF5,0x5<<5) # 50ms. meas standby
    w_reg(0xF4,0x2 | (0x2 << 5)) # set barometer to forced mode

def getCals():
    addr = 0x88
    for x in range(0,3):
        temp = r_reg(addr,2)
        tempCal[x] = (float)(temp[1] << 8 | temp[0] & 0xff ) 
        addr+=0x2
        #print(tempCal[x])

    for x in range(0,9):
        pres = r_reg(addr,2)
        presCal[x] = (float)(pres[1] << 8 | pres[0] & 0xff ) 
        addr+=0x2
        #print(presCal[x])
    #exit()

def read_temp():
    
    force()
    
    while(r_reg(0xf3,1)[0] & 0x08 != 0):
        print("spinning...")
        cs(1)
        
    t_b = r_reg(0xfa,3)
    
    print("%x %x %x" %( t_b[0], t_b[1], t_b[2]))
    
    raw_temp = (float)((((t_b[0]<<16) | (t_b[1] << 8) | (t_b[2] & 0xFF)) & 0xffffff)/16)
    
    
    
    
    temp1 = (raw_temp/16384.0 - tempCal[0] / 1024.0) *tempCal[1]
    
    temp2 = (raw_temp /131072.0 - tempCal[0] / 8192.0) * (raw_temp /131072.0 - tempCal[0] / 8192.0) * tempCal[2]
    
    t_fine = (float)(temp1+temp2)
   # print("%f" % raw_temp)


    return (t_fine/5120.0)

    normal()
    
def read_pres():
    
    p_b = r_reg(0xf7,3)
    
    print(p_b)
    
    raw_pres = (float)((((p_b[0]<<16) | (p_b[1] << 8) | (p_b[2] & 0xFF)) & 0xffffff)/16)
    
    v1 = float(t_fine/2.0) - 64E3
    
    v2 = v1 * v1 * presCal[5] / 32768.0
    v2 = v2 + v1 * presCal[4] * 2.0
    v2 = v2 / 4.0 + presCal[3] * 65536.0
    
    v3 = presCal[2] * v1 * v1 / 524288.0
    
    v1 = (v3 + presCal[1] * v1) / 524288.0
    v1 = (1.0 + v1 / 32768.0) * presCal[0]
    
    pressure = 1048576.0 - raw_pres
    pressure = ((pressure - v2 / 4096.0) * 6250.0) / v1
    
    v1 = presCal[8] * pressure * pressure / 2147483648.0
    v2 = pressure * presCal[7] / 32768.0
    
    pressure = pressure + (v1 + v2 + presCal[6]) / 16.0
    pressure /= 100
    
    return pressure 
    



              
spi = SPI(0,
          baudrate = 100000,
          polarity = 0,
          phase = 0,
          bits = 8,
          sck = Pin(2),
          mosi = Pin(3),
          miso = Pin(4))
    
    
cs = Pin(5,Pin.OUT,value=0) #init value as 0 to disable i2c


cs(1)

reset()


normal()

utime.sleep(0.1)

getCals()



while(True):
    print(read_temp()*1.8 +32)
    print(read_pres())
    utime.sleep(0.5)
