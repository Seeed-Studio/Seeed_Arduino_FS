#include <Seeed_FS.h>
#include "SFUD/Seeed_SFUD.h"
#include "SD/Seeed_SD.h"
#define SERIAL Serial

#ifdef _SAMD21_
#define SDCARD_SS_PIN 2
#define SDCARD_SPI SPI
#endif 

void listDir(fs::FS& fs,const char* dirname, uint8_t levels) {
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
void setup() {
    SERIAL.begin(115200);
    while (!SERIAL) {};
#ifdef SFUD_USING_QSPI
    while (!SPIFLASH.begin(104000000UL)) {
        SERIAL.println("Flash Mount Failed");
        return;
    }
    while (!SD.begin(SDCARD_SS_PIN,SDCARD_SPI,4000000UL)) {
        SERIAL.println("SD Card Mount Failed");
        return;
    }
#else
    while (!SD.begin(SDCARD_SS_PIN,SDCARD_SPI,4000000UL)) {
        SERIAL.println("SD Card Mount Failed");
        return;
    }
    while (!SPIFLASH.begin(1,SPI,4000000UL)) {
        SERIAL.println(" Flash Mount Failed");
        return;
    }    
#endif
    uint8_t flashType = SPIFLASH.flashType();
    if (flashType == FLASH_NONE) {
        SERIAL.println("No Flash attached");
        return;
    }
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        SERIAL.println("No SD card attached");
        return;
    }
    uint8_t flashSize = SPIFLASH.flashSize() / (1024 * 1024);
    SERIAL.print("Flash Size: ");
    SERIAL.print((uint32_t)flashSize);
    SERIAL.println("MB");
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    SERIAL.print("SD Card Size: ");
    SERIAL.print((uint32_t)cardSize);
    SERIAL.println("MB");

    uint32_t FlashtotalBytes = SPIFLASH.totalBytes();
    SERIAL.print("Total space: ");
    SERIAL.print(FlashtotalBytes / (1024 * 1024));
    SERIAL.println("MB");
    uint32_t FlasusedBytes = SPIFLASH.usedBytes();
    SERIAL.print("Used space: ");
    SERIAL.print(FlasusedBytes / (1024 * 1024));
    SERIAL.println("MB");

    uint32_t SDtotalBytes = SD.totalBytes();
    SERIAL.print("Total space: ");
    SERIAL.print(SDtotalBytes / (1024 * 1024));
    SERIAL.println("MB");
    uint32_t SDusedBytes = SD.usedBytes();
    SERIAL.print("Used space: ");
    SERIAL.print(SDusedBytes / (1024 * 1024));
    SERIAL.println("MB");

    listDir(SD,"1:/",0);
    listDir(SPIFLASH,"0:/",0);

}

void loop() {
    
}