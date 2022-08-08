import java.awt.image.BufferedImage;
import java.io.File;
import javax.imageio.ImageIO;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.ArrayList;

class Obj2H {
    static boolean doquad;
    static RandomAccessFile r;
    static RandomAccessFile w;

    static int vertcount;
    static int quadcount;
    static int tricount;
    static ArrayList<float[]> verts;
    static ArrayList<Integer[]> quads;
    static ArrayList<Integer[]> tris;

    static float scalar = 100f;

    static float[] scale(float[] vert) {

        for (int i = 0; i < vert.length; ++i)
            vert[i] *= scalar;

        return vert;

    }

    public static float[] getVert(String get) {
        float[] xyz = new float[3];

        String[] vert = get.split(" ");

        for (int i = 1; i < vert.length; ++i)
            xyz[i - 1] = Float.parseFloat(vert[i]);

        /*
         * for (float f : xyz)
         * System.out.println(f);
         * System.out.printf("\n");
         */

        return scale(xyz);

    }

    public static Integer[] getQuad(String get) {

        Integer[] abcd = new Integer[4];

        String[] quad = get.split(" ");

        for (int i = 1; i < quad.length; ++i)
            abcd[i - 1] = Integer.parseInt(quad[i])-1;

        /*
         * for (float f : xyz)
         * System.out.println(f);
         * System.out.printf("\n");
         */
        return abcd;

    }

    public static Integer[] getTri(String get) {

        Integer[] abc = new Integer[4];

        String[] tri = get.split(" ");

        for (int i = 1; i < tri.length; ++i)
            abc[i - 1] = Integer.parseInt(tri[i])-1;

        /*
         * for (float f : xyz)
         * System.out.println(f);
         * System.out.printf("\n");
         */
        return abc;

    }

    public static void wh(String type, String name) throws IOException {

        switch (type) {

            case "u32":
                type = "uint32_t";
                break;
            case "u16":
                type = "uint16_t";
                break;
            case "16":
                type = "int16_t";
                break;
            default:
                type = "uint8_t";
        }

        char[] str = String.format("\nconst %s %s[] = {\n", type, name)
                .toCharArray();

        for (char c : str)
            w.writeByte(c);

    }

    public static void write(String txt) throws IOException {
        char[] l = txt.toCharArray();

        for (char x : l)
            w.writeByte(x);
    }

    public static void endArray() throws IOException {
        write("};\n\n");
    }

    public static void main(String[] args) throws IOException {

        doquad = false;

        if(args[1].equals("quad"))
            doquad =true;

        String name = args[0];

        if (name.equals(null))
            name = "model";

        File i = new File("monkey.obj");
        File o = new File(String.format("%s.h", name));

        o.delete();

        r = new RandomAccessFile(i, "r");
        w = new RandomAccessFile(o, "rw");

        // char[] begin = "#include \"pico/stdlib.h\"\n#include
        // \"pico/binary_info.h\"\n#include <stdio.h>\n\n\nconst uint32_t __in_flash()
        // test_img2[] = {\n"
        // .toCharArray();

        verts = new ArrayList<float[]>();
        quads = new ArrayList<Integer[]>();
        tris = new ArrayList<Integer[]>();

        String temp = "";
        boolean found = false;
        boolean exit = false;

        // read verts///////////////////////////////////////////////////////////////////

        while (!exit) {

            temp = r.readLine();
            if (temp.charAt(0) == 'v') {
                verts.add(getVert(temp));
                found = true;
            } else if (found == true)
                exit = true;

        }

        vertcount = verts.size();

        p("vert count: " + vertcount);

        // write verts///////////////////////////////////////

        p("writing verts");

        write(String.format("const uint16_t num%sverts = %d;\n", name, vertcount));
        wh("16", String.format("vert%s", name));

        int index = 0;

        for (float[] vert : verts) {
            write(String.format("%d,%d,%d", Math.round(vert[0]), Math.round(vert[1]), Math.round(vert[2])));
            index++;
            if (index != vertcount)
                write(",\n");
            else
                endArray();
        }

        System.out.println(r.getFilePointer());
        if(doquad) {
          // read quads/////////////////////////////////////
          
          found = false;
          exit = false;
          
          while (!exit) {
          
          temp = r.readLine();
          if (temp != null) {
          if (temp.charAt(0) == 'f') {
          
          quads.add(getQuad(temp));
          found = true;
          } else if (found == true)
          exit = true;
          } else
          exit = true;
          }
          
          // write quads //////////////////////////////////
          
          quadcount = quads.size();
          
          write(String.format("const uint16_t num%squads = %d;\n", name, quadcount));
          wh("u8", String.format("quad%s", name));
          
          index = 0;
          
          for (Integer[] quad : quads) {
          write(String.format("%d,%d,%d,%d", quad[0], quad[1], quad[2], quad[3]));
          index++;
          if (index != quadcount)
          write(",\n");
          else
          endArray();
          }
         
        }
        else{
        // read tris/////////////////////////////////////

        found = false;
        exit = false;

        while (!exit) {

            temp = r.readLine();
            if (temp != null) {
                if (temp.charAt(0) == 'f') {

                    tris.add(getTri(temp));
                    found = true;
                } else if (found == true)
                    exit = true;
            } else
                exit = true;
        }

        // write tris //////////////////////////////////

        tricount = tris.size();

        write(String.format("const uint16_t num%stris = %d;\n", name, tricount));
        wh("u8", String.format("tri%s", name));

        index = 0;

        for (Integer[] tri : tris) {
            write(String.format("%d,%d,%d", tri[0], tri[1], tri[2]));
            index++;
            if (index != tricount)
                write(",\n");
            else
                endArray();
        }

    }


    /////colors///////////////

    int ccount = doquad ? quadcount : tricount;

        wh("u16", String.format("cols%s", name));

        int loop = 3;

        for (int p = 0; p < ccount; ++p)
            write(String.format("COL3%d,\n", --loop < 1 ? loop += 3 : loop));
        endArray();


        ///////extra

        if(!doquad) {

        write(String.format("const uint16_t num%squads = %d;\n", name, 0));
        wh("u8", String.format("quad%s", name));
        endArray();


        }
        else {

            write(String.format("const uint16_t num%verts = %d;\n", name, 0));
            wh("u8", String.format("vert%s", name));
            endArray();
        }

        System.out.println("done.");

    }

    static void p(String txt) {
        System.out.printf("%s\n", txt);
    }
}