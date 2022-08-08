#include <windows.h>
#include <stdio.h>
#include <iostream> 
#include "serialib.h"
#include <string.h>

#define SERIAL_PORT "\\\\.\\COM11"

HANDLE serial;
DWORD dwRead;
BOOL fWaitingOnRead;
OVERLAPPED osReader;


void error(std::string c) {
    printf("%s",c.c_str());
    
}

int main(){


char gunk[4]={0}; 



serial = CreateFile(SERIAL_PORT,GENERIC_READ|GENERIC_WRITE,0,0,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,0);


if(serial == INVALID_HANDLE_VALUE)
    error("Couldn't create handle.");

fWaitingOnRead = FALSE;
osReader = {0};

osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

if(osReader.hEvent == NULL)
    error("Couldn't create overlapped evt.");


if(!fWaitingOnRead) {

    if(!ReadFile(serial,gunk,4,&dwRead,&osReader)){
        if (GetLastError() != ERROR_IO_PENDING)
            error("couldn't read");

        else
            fWaitingOnRead = TRUE;
    }
    else {
        printf("Read %d bytes.\n",dwRead);
    }
}







}