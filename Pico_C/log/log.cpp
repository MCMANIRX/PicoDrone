#include "serialib.cpp"
#include "to888.h"
#include <synchapi.h>
#include <opencv2/opencv.hpp>
#include <vector>
#define SERIAL_PORT "\\\\.\\COM11"

using namespace cv;

    char errorOpening;
    serialib serial;
    cv::Mat img;





void retry() {
            serial.closeDevice();
        Sleep(1000);
         errorOpening = serial.openDevice(SERIAL_PORT, 115200);
        printf("Failure to connect to COM PORT. Retrying.\n");

}


int readLine(char buf[]) {

    char c = 0;
    int index = 0;
    int count = 0 ;


    while(c!='p')
       serial.readChar(&c);

    while(1){
        count += serial.readChar(&c);
        if(c=='\n')break;
        buf[index++] = c;

    }

 /*  for(int i = 0; i <index; ++i)
            printf("%x ",(buf[i]));
    std::cout << "\n";*/

            return count;


}

int main() {


    uint32_t frame[76800]={0};



       errorOpening = serial.openDevice(SERIAL_PORT, 115200);



    // If connection fails, return the error code otherwise, display a success message
    while (errorOpening!=1) retry();



    printf("Successful connection to %s\n",SERIAL_PORT);


        // Set DTR
    serial.DTR(true);
    // Clear RTS
    serial.RTS(false);

    char x = 's';
    char gunk[4]= {0};



/*int count = 0;
    while(x!='S'){
        std::cout << "waiting for 'S'...\n";
        serial.readChar(&x);
        count++;
            std::cout << x;

        if(count >10){
            retry();
            Sleep(500);
            count = 0;}
    }
    printf("'S' found.\n");
*/
    while(x!='s'){
        serial.readChar(&x);

        if(x=='s')break;
    }

        printf("'s' found.\n");


    while(x=='s'){
        serial.readChar(&x);
        std::cout <<  "Synching...\n";
    }

readLine(gunk);

   printf("Beginning frame read....\n");


while(1){
    readLine(gunk);
      if (strcmp(gunk,"one")==0 )break;;

       //   for(int j = 0; j < sizeof(gunk)/sizeof(char); ++j)
        printf("%s",gunk);
      //  std::cout << "\n";

    }
    printf("One!");


    int index  = 0;
    int count1 = 0;

    while(index<76800*4) {
       count1+= readLine(gunk);

       std::cout << count1;

  
      // for(int i = 0 ; i < count1; ++i)
      //  frame[index+i] = to888((uint16_t)(std::strtol(gunk,NULL,16)));
               // printf("%s,\t%x\n",gunk,frame[index+i]);

        index+=count1;
        count1=0;

             // if (strcmp(gunk,"end")==0 )break;;


    }

    printf("Recieved frame 1.\n");

    for(int j = 0; j < sizeof(gunk)/sizeof(uint32_t); ++j)
        printf("%x",gunk[j]);
        std::cout << "\n";

    Mat img(240,320,CV_8UC4,frame);






    cv::namedWindow("pico", cv::WINDOW_AUTOSIZE);
    cv::imshow("pico",img);
   cv::waitKey(0); //&0xff;


 


    



   

    return 0;
}