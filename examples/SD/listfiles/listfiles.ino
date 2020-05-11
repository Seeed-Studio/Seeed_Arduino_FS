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
#include <Seeed_FS.h>



#ifdef USESPIFLASH
#define DEV SPIFLASH
#include "SFUD/Seeed_SFUD.h"
#else
#define DEV SD
#include "SD/Seeed_SD.h"
#endif 

#define SERIAL Serial
File root;

#ifdef _SAMD21_
#define SDCARD_SS_PIN 1
#define SDCARD_SPI SPI
#endif 

void setup() {
    // Open SERIAL communications and wait for port to open:
    SERIAL.begin(115200);
    while (!SERIAL) {
        ; // wait for SERIAL port to connect. Needed for native USB port only
    }

    SERIAL.print("Initializing SD card...");

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

    listDir(DEV, "/", 0);

    SERIAL.println("done!");
}

void loop() {
    // nothing happens after setup finishes.
}
void listDir(fs::FS& fs, const char* dirname, uint8_t levels) {
    SERIAL.print("Listing directory: ");
    SERIAL.println(dirname);

    File root = fs.open(dirname);
    if (!root) {
        SERIAL.println("Failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        SERIAL.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            SERIAL.print("  DIR : ");
            SERIAL.println(file.name());
            if (levels) {
                listDir(fs, file.name(), levels - 1);
            }
        } else {
            SERIAL.print("  FILE: ");
            SERIAL.print(file.name());
            SERIAL.print("  SIZE: ");
            SERIAL.println(file.size());
        }
        file = root.openNextFile();
    }
}



