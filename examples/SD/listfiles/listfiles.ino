/*
    Listfiles

    This example shows how print out the files in a
    directory on a SD card

    The circuit:
    SD card attached to SPI bus as follows:
 ** MOSI - MOSI
 ** MISO - MISO
 ** CLK - CLK
 ** CS - 11 (for MKRZero SD: SDCARD_SS_PIN)

    created   Nov 2010
    by David A. Mellis
    modified 9 Apr 2012
    by Tom Igoe
    modified 2 Feb 2014
    by Scott Fitzgerald
    modified 15 July 2019
    by Hongtai.liu

    This example code is in the public domain.

*/
#ifdef KENDRYTE_K210
    #include <SPIClass.h>
#else
    #include <SPI.h>
#endif
#include <Seeed_FS.h>
#include "SD/Seeed_SD.h"

#define SERIAL Serial

#define csPin 4

File root;



void setup() {
    // Open SERIAL communications and wait for port to open:
    SERIAL.begin(115200);
    while (!SERIAL) {
        ; // wait for SERIAL port to connect. Needed for native USB port only
    }

    SERIAL.print("Initializing SD card...");

    if (!SD.begin(csPin)) {
        SERIAL.println("initialization failed!");
        while (1);
    }
    SERIAL.println("initialization done.");

    root = SD.open("/");

    printDirectory(root, 0);

    SERIAL.println("done!");
}

void loop() {
    // nothing happens after setup finishes.
}

void printDirectory(File dir, int numTabs) {
    while (true) {

        File entry =  dir.openNextFile();
        if (! entry) {
            // no more files
            break;
        }
        for (uint8_t i = 0; i < numTabs; i++) {
            SERIAL.print('\t');
        }
        SERIAL.print(entry.name());
        if (entry.isDirectory()) {
            SERIAL.println("/");
            printDirectory(entry, numTabs + 1);
        } else {
            // files have sizes, directories do not
            SERIAL.print("\t\t");
            SERIAL.println(entry.size(), DEC);
        }
        entry.close();
    }
}



