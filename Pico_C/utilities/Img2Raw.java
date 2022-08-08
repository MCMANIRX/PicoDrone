import java.awt.image.BufferedImage;
import java.io.File;
import javax.imageio.ImageIO;
import java.io.IOException;
import java.io.RandomAccessFile;

class Img2Raw {

    static BufferedImage in;
    static RandomAccessFile w;
    static boolean print8;

    public static void main(String[] args) throws IOException {
        print8 = false;

        if (args.length > 0)
            if (args[0].equals("b"))
                print8 = true;

        File i = new File(args[0]);
        File o = new File("t_image.h");

        o.delete();

        int wi;
        int hi;

        in = ImageIO.read(i);
        w = new RandomAccessFile(o, "rw");

        wi = in.getWidth();
        hi = in.getHeight();

        int[] buf = new int[wi * hi];

        for (int x = 0; x < wi; ++x)
            for (int y = 0; y < hi; ++y)
                buf[(y * wi) + x] = in.getRGB(x, y);

        char[] begin = "#include \"pico/stdlib.h\"\n#include \"pico/binary_info.h\"\n#include <stdio.h>\n\n\nconst uint32_t __in_flash() test_img[] = {\n"
                .toCharArray();

        for (char c : begin)
            w.writeByte(c);

        char[] temp;

        int num = 0;
        int nl = 0;

        if (print8) {
            for (Integer c : buf) {
                nl++;
                num++;
                for (int t = 0; t < c.BYTES; ++t) {
                    temp = String.format("0x%x,", (c >> (t * 4)) & 0xff).toCharArray();
                    for (char ch : temp)
                        w.writeByte(ch);
                }

                if (nl == 10) {
                    w.writeBytes("\n");
                    nl = 0;
                }

            }

        } else
            for (int c : buf) {
                w.writeBytes("0x");
                num++;
                nl++;
                for (int k = 5; k > -1; --k) {
                    temp = String.format("%s", Integer.toHexString(c >> (4 * k) & 0xf)).toCharArray();
                    for (char ch : temp)
                        w.writeByte(ch);

                }

                if (num < (wi * hi))
                    w.writeByte(',');

                if (nl == 10) {
                    w.writeBytes("\n");
                    nl = 0;
                }
            }

        System.out.println(num);/*
                                 * for (int k = 2; k > -1; --k) {
                                 * temp = String.format("0x%s", Integer.toHexString(c >> (8 * k) &
                                 * 0xff)).toCharArray();
                                 * for (char ch : temp)
                                 * w.writeByte(ch);
                                 * num++;
                                 * if(num<wi*hi*3)
                                 * w.writeByte(',');
                                 * 
                                 * }
                                 */

        char[] end = "};".toCharArray();
        for (char ch : end)
            w.writeByte(ch);

        System.out.println("done.");

    }
}