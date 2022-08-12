import serial
import cv2 
import numpy as np


cc58 =  [
	    0x00,0x08,0x10,0x19, 0x21,0x29,0x31,0x3a, 0x42,0x4a,0x52,0x5a, 0x63,0x6b,0x73,0x7b,
	    0x84,0x8c,0x94,0x9c, 0xa5,0xad,0xb5,0xbd, 0xc5,0xce,0xd6,0xde, 0xe6,0xef,0xf7,0xff
    ]

cc68 = [
		    0x00,0x04,0x08,0x0c, 0x10,0x14,0x18,0x1c, 0x20,0x24,0x28,0x2d, 0x31,0x35,0x39,0x3d,
		    0x41,0x45,0x49,0x4d, 0x51,0x55,0x59,0x5d, 0x61,0x65,0x69,0x6d, 0x71,0x75,0x79,0x7d,
		    0x82,0x86,0x8a,0x8e, 0x92,0x96,0x9a,0x9e, 0xa2,0xa6,0xaa,0xae, 0xb2,0xb6,0xba,0xbe,
		    0xc2,0xc6,0xca,0xce, 0xd2,0xd7,0xdb,0xdf, 0xe3,0xe7,0xeb,0xef, 0xf3,0xf7,0xfb,0xff
    ]

h=240
w=320

frame = cv2.imread("blank.png")#// np.zeros((h,w,3),np.uint8)


begin = False

x='a'

framebuf = []

framebuf16 = []


print("Waiting for PICO....")

while(not begin):
    try:

        com = serial.Serial(
            port = 'COM14',

            baudrate = 115200,

            parity = serial.PARITY_NONE,

            stopbits = serial.STOPBITS_ONE,

            bytesize = serial.EIGHTBITS,

            timeout = 1
        )
        x = com.read(1)
        print(x)
        

    
    except:
        begin = begin

    if x == b'S':
        begin = True
        print("begin read")
        print(com.read(1))

        print(com.read(1))

        print(com.read(1))

        print(com.read(1))

        print(com.read(1))

        break 

while(com.read(1)!=b's'):
    begin =begin

while(com.read(1)==b's'):
    begin = begin
    print("spinning...")


f1 = False
f2 = False
f3 = False
f4 = False
#com.read(1) #remove null


one = []
two = []
three = []
four = []


b = 0


# 00 39 00 c8 fix!!

while(not f1):
    x = com.readline()
  #  print(x)
  #  print(len(one))


    if(x==b'one\n' and f1 == False):
        print("found one")
        while(b!=b'end\n'):
            b = com.readline()
            if(len(b)>5):
                one = []
                print("read failed. retrying...")
                break
            one.append(b)
        f1 = True
        break
    '''   # com.read(1)
        while(True):
            b = com.readline()
            one.append(b)
       #     print(b)
            if(b == 'end\n' ):
                break;
                f1=True

'''
       # if(len(one)>76800):

         

'''   if(x==b'two\n' and f2 == False):
        com.read(1)
        two = com.read(19200)

        print("found tew")
        if(len(two)==19200):
            f2 = True
        

    if(x==b'thr\n' and f3 == False):
        com.read(1)
        three = com.read(19200)
        
        print("found three")
        if(len(three)==19200):
            f3 = True

    if(x==b'for\n'and f4 == False):
        com.read(1)
        four = com.read(19200)
                
        print("found four")
        if(len(four)==19200):
            f4 = True

    if(f1 and f2 and f3 and f4):
        break
'''


#print((one))
one.pop()

for byt in one:
    framebuf.append(int(byt[:-1],16))
    
    
"""for byt in two:
    framebuf.append(byt)
for byt in three:
    framebuf.append(byt)
for byt in four:
    framebuf.append(byt)"""

#print(framebuf)

"""while(1):

   # y = com.read(1)

   #if(y==b'p'):

    x = com.read(2)
    framebuf.append(int.from_bytes(x,"big"))


   # print(len(framebuf))
    print(x)

    if(x== b'e'):
        #framebuf.pop()
        break
   # try:
"""



print("done")         
print(len(framebuf))

out = open("log/log.bin","wb")

for short in framebuf:
    out.write(bytearray([(short>>8)&0xff]))
    out.write(bytearray([(short)&0xff]))

out.close()

for i in range(0,h):
    for j in range(0,w):
        
        b = framebuf[(w*i) + j]
        frame[i,j] = (cc58[b&0x1f],cc68[(b>>5)&0x3f],cc58[(b>>11)&0x1f])


cv2.imshow('pico image',frame)
cv2.imwrite('frame.png',frame)
cv2.waitKey(0)



com.close()
exit()




           
 #   except:
 #       print("Waiting for connection...")
 #       com = serial.Serial(
 #           port = 'COM11',

 #           baudrate = 115200,

  #          parity = serial.PARITY_NONE,

  #          stopbits = serial.STOPBITS_ONE,

  #          bytesize = serial.EIGHTBITS,

  #          timeout = 1
  #      )
  
