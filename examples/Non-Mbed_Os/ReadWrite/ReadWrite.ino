/*
    SD card read/write

    This example shows how to read and write data to and from an SD card file
    The circuit:
    SD card attached to SPI bus or SDMMC

    created   Nov 2010
    by David A. Mellis
    modified 9 Apr 2012
    by Tom Igoe
    modified 18 June 2021
    by Hongtai.liu

    This example code is in the public domain.

*/
#include <Seeed_Arduino_FS.h>


#ifdef WIO_LITE_AI
#define DEV  SDMMC 
#else
#define DEV  SD
#ifdef _SAMD21_
#define SDCARD_SS_PIN 1
#define SDCARD_SPI SPI
#endif 
#endif


#define LOG Serial

void setup() {
    LOG.begin(115200);
    pinMode(5, OUTPUT);
    digitalWrite(5, HIGH);
    while (!LOG) {};
#ifdef WIO_LITE_AI
    while (!DEV.begin()) {
        LOG.println("Card Mount Failed");
        return;
    }  
#else
    while (!DEV.begin(SDCARD_SS_PIN,SDCARD_SPI,4000000UL)) {
        LOG.println("Card Mount Failed");
        return;
    }  
#endif
    LOG.println("initialization done.");

    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.

    File RootWrite = DEV.open("/hello.txt", "w");
    // File RootWrite = DEV.open("/hello.txt", FILE_WRITE);

    // if the file opened okay, write to it:
    if (RootWrite) {
        LOG.print("Writing to hello.txt...");
        RootWrite.println("hello 1, 2, 3.");
        // close the file:
        RootWrite.close();
        LOG.println("done.");
    } else {
        // if the file didn't open, print an error:
        LOG.println("error opening hello.txt");
    }
    
    // re-open the file for reading:
    File RootRead= DEV.open("/hello.txt");
    if (RootRead) {
        LOG.println("hello.txt:");

        // read from the file until there's nothing else in it:
        while (RootRead.available()) {
            LOG.write(RootRead.read());
        }
        // close the file:
        RootRead.close();
    } else {
        // if the file didn't open, print an error:
        LOG.println("error opening hello.txt");
    }
}

void loop() {
    // nothing happens after setup
}


