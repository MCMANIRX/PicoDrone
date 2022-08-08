_in = open("jpeg.jpeg","rb")
out = open("test.h","wb")

out.write(b'const uint8_t test_img[] = {\n')



count = 0

one = False

while(True):
    b = _in.read(1)
    if not b:
        out.write(b'};')
        break
    elif(one):
        out.write(b',')
    out.write(bytearray(hex(b[0]).encode('utf-8')))
    one = True
    count+=1
    if count == 10:
        out.write(b'\n')
        count = 0
    
print("done.")