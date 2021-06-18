/*
    Listfiles

    This example shows how print out the files in a
    directory on a SD card

    The circuit:
    SD card attached to SPI bus as follows:


    created   Nov 2010
    by David A. Mellis
    modified 9 Apr 2012
    by Tom Igoe
    modified 2 Feb 2014
    by Scott Fitzgerald
    modified 18 July 2021
    by Hongtai.liu

    This example code is in the public domain.

*/
#include <Seeed_Arduino_FS.h>

#define DEV SD
#define LOG Serial

File root;

#ifdef _SAMD21_
#define SDCARD_SS_PIN 1
#define SDCARD_SPI SPI
#endif 

void setup() {
    // Open LOG communications and wait for port to open:
    LOG.begin(115200);
    while (!LOG) {
        ; // wait for LOG port to connect. Needed for native USB port only
    }

    LOG.print("Initializing SD card...");

    while (!DEV.begin(SDCARD_SS_PIN,SDCARD_SPI,4000000UL)) {
        LOG.println("Card Mount Failed");
        return;
    }

    LOG.println("initialization done.");

    listDir(DEV, "/", 0);

    LOG.println("done!");
}

void loop() {
    // nothing happens after setup finishes.
}
void listDir(fs::FS& fs, const char* dirname, uint8_t levels) {
    LOG.print("Listing directory: ");
    LOG.println(dirname);

    File root = fs.open(dirname);
    if (!root) {
        LOG.println("Failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        LOG.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            LOG.print("  DIR : ");
            LOG.println(file.name());
            if (levels) {
                listDir(fs, file.name(), levels - 1);
            }
        } else {
            LOG.print("  FILE: ");
            LOG.print(file.name());
            LOG.print("  SIZE: ");
            LOG.println(file.size());
        }
        file = root.openNextFile();
    }
}



