import java.io.FileWriter;
import java.awt.Image;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.RandomAccessFile;

import javax.imageio.ImageIO;
import java.io.IOException;
import java.util.ArrayList;
import java.io.EOFException;

class JPEGtoH {

    static int len = 0;

    static BufferedImage in;
    static FileWriter w;

    public static void main(String[] args) throws IOException {

        File i = new File("tae.jpg");
        File o = new File("t_image.h");

        ArrayList<Character> str = new ArrayList<Character>();

        RandomAccessFile r = new RandomAccessFile(i, "r");
        RandomAccessFile w = new RandomAccessFile(o, "rw");

        len = (int) i.length();

        // w = new FileWriter(o);

        char[] begin = "const uint8_t test_img[] = {\n".toCharArray();

        for (char c : begin)
            w.writeByte(c);

        String temp = "";
        while (true) {

            temp = String.format("0x%x", r.readByte());
            w.writeBytes(temp);

            if ((int) r.getFilePointer() != len)
                w.writeBytes(", ");
            else
                break;

        }

        char[] end = "};".toCharArray();

        w.writeBytes(new String(end));

        // for (char x : wstr)
        // System.out.print(x);
        System.out.println("done.");

        w.close();
    }
}