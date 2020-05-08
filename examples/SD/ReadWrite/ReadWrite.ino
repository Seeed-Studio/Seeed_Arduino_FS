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

#define SERIAL Serial

#ifdef USESPIFLASH
#define DEV SPIFLASH
#include "SFUD/Seeed_SFUD.h"
#else
#define DEV SD
#include "SD/Seeed_SD.h"
#endif 

#define csPin 4
#ifdef ARDUINO_ARCH_SAMD
    #undef SERIAL Serial
    #define SERIAL SerialUSB
#endif

File ROOT;

void setup() {
    SERIAL.begin(115200);
    pinMode(5, OUTPUT);
    digitalWrite(5, HIGH);
    while (!SERIAL) {};
#ifdef SFUD_USING_QSPI
    while (!DEV.begin(104000000UL)) {
        SERIAL.println("Card Mount Failed");
        return;
    }
#else
    while (!DEV.begin(SDCARD_SS_PIN,SDCARD_SPI,4000000UL)) {
        SERIAL.println("Card Mount Failed");
        return;
    }
#endif 
    SERIAL.println("initialization done.");

    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    ROOT = DEV.open("test.txt", FILE_WRITE);

    // if the file opened okay, write to it:
    if (ROOT) {
        SERIAL.print("Writing to test.txt...");
        ROOT.println("testing 1, 2, 3.");
        // close the file:
        ROOT.close();
        SERIAL.println("done.");
    } else {
        // if the file didn't open, print an error:
        SERIAL.println("error opening test.txt");
    }

    // re-open the file for reading:
    ROOT = DEV.open("test.txt");
    if (ROOT) {
        SERIAL.println("test.txt:");

        // read from the file until there's nothing else in it:
        while (ROOT.available()) {
            SERIAL.write(ROOT.read());
        }
        // close the file:
        ROOT.close();
    } else {
        // if the file didn't open, print an error:
        SERIAL.println("error opening test.txt");
    }
}

void loop() {
    // nothing happens after setup
}


