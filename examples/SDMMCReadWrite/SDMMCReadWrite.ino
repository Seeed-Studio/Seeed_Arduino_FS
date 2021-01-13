/*
    SD card read/write

    This example shows how to read and write data to and from an SD card file
    The circuit:
    SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)

    created   Nov 2010
    by David A. Mellis
    modified 9 Apr 2012
    by Tom Igoe

    This example code is in the public domain.

*/

#include <Seeed_FS.h>
#include "SDMMC/Seeed_SDMMC.h"

#define DEV  SDMMC

SDMMCFS SDMMC;


void setup() {
    printf("initialization done..\r\n");
    if (!DEV.begin(0,4000000UL)) {
        printf("initialization error.\r\n");
    }
    printf("initialization done.\r\n");
    //  FIL file;
    //  if (f_open(&file, "0:hello.txt", FA_CREATE_ALWAYS | FA_WRITE | FA_READ) == FR_OK){
    //      printf("open to hello.txt ok\r\n");
    //  }else
    //  {
    //       printf("open to hello.txt error\r\n");
    //  }
     
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    #if 1
    File RootWrite = DEV.open("0:/hello.txt", "w+");
    // File RootWrite = DEV.open("/hello.txt", FILE_WRITE);

    // if the file opened okay, write to it:
    if (RootWrite) {
        printf("Writing to hello.txt...\r\n");
        RootWrite.println("hello 1, 2, 3.");
        // close the file:
        RootWrite.close();
        printf("done.\r\n");
    } else {
        // if the file didn't open, print an error:
        printf("error opening hello.txt\r\n");
    }
    
    // re-open the file for reading:
    File RootRead= DEV.open("0:/hello.txt");
    if (RootRead) {
        printf("hello.txt:");

        // read from the file until there's nothing else in it:
        while (RootRead.available()) {
            printf("%x ", RootRead.read());
        }
        // close the file:
        RootRead.close();
    } else {
        // if the file didn't open, print an error:
        printf("error opening hello.txt\r\n");
    }
    #endif
}

void loop() {
    // nothing happens after setup
}


