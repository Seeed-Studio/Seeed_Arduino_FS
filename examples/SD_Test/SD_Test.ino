/*
    Connect the SD card to the following pins:

    SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)
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

#ifdef _SAMD21_
#define SDCARD_SS_PIN 1
#define SDCARD_SPI SPI
#endif 


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

void createDir(fs::FS& fs, const char* path) {
    SERIAL.print("Creating Dir: ");
    SERIAL.println(path);
    if (fs.mkdir(path)) {
        SERIAL.println("Dir created");
    } else {
        SERIAL.println("mkdir failed");
    }
}

void removeDir(fs::FS& fs, const char* path) {
    SERIAL.print("Removing Dir: ");
    SERIAL.println(path);
    if (fs.rmdir(path)) {
        SERIAL.println("Dir removed");
    } else {
        SERIAL.println("rmdir failed");
    }
}

void readFile(fs::FS& fs, const char* path) {
    SERIAL.print("Reading Dir: ");
    SERIAL.println(path);
    File file = fs.open(path);
    if (!file) {
        SERIAL.println("Failed to open file for reading");
        return;
    }

    SERIAL.print("Read from file: ");
    while (file.available()) {
        SERIAL.write(file.read());
    }
    file.close();
}

void writeFile(fs::FS& fs, const char* path, const char* message) {
    SERIAL.print("Writing file: ");
    SERIAL.println(path);
    File file = fs.open(path, FILE_WRITE);
    if (!file) {
        SERIAL.println("Failed to open file for writing");
        return;
    }
    
    if (file.print(message)) {
        SERIAL.println("File written");
    } else {
        SERIAL.println("Write failed");
    }

    file.close();

}

void appendFile(fs::FS& fs, const char* path, const char* message) {
    SERIAL.print("Appending to file: ");
    SERIAL.println(path);

    File file = fs.open(path, FILE_APPEND);
    if (!file) {
        SERIAL.println("Failed to open file for appending");
        return;
    }
    if (file.print(message)) {
        SERIAL.println("Message appended");
    } else {
        SERIAL.println("Append failed");
    }
    file.close();
}

void renameFile(fs::FS& fs, const char* path1, const char* path2) {
    SERIAL.print("Renaming file ");
    SERIAL.print(path1);
    SERIAL.print(" to ");
    SERIAL.println(path2);
    if (fs.rename(path1, path2)) {
        SERIAL.println("File renamed");
    } else {
        SERIAL.println("Rename failed");
    }
}

void deleteFile(fs::FS& fs, const char* path) {
    SERIAL.print("Deleting file: ");
    SERIAL.println(path);
    if (fs.remove(path)) {
        SERIAL.println("File deleted");
    } else {
        SERIAL.println("Delete failed");
    }
}

void testFileIO(fs::FS& fs, const char* path) {
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = micros();
    uint32_t end = start;
    if (file) {
        len = file.size();
        size_t flen = len;
        start = micros();
        while (len) {
            size_t toRead = len;
            if (toRead > 512) {
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = micros() - start;
        SERIAL.print(flen);
        SERIAL.print(" bytes read for ");
        SERIAL.print(end);
        SERIAL.println(" ns");
        file.close();
    } else {
        SERIAL.println("Failed to open file for reading");
    }
}

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


#ifdef USESPIFLASH
    uint8_t flashType = DEV.flashType();
    if (flashType == FLASH_NONE) {
        SERIAL.println("No flash attached");
        return;
    }
#else
    uint8_t cardType = DEV.cardType();
    if (cardType == CARD_NONE) {
        SERIAL.println("No SD card attached");
        return;
    }
#endif 

#ifdef USESPIFLASH
    uint32_t flashSize = DEV.flashSize() / (1024 * 1024);
    SERIAL.print("flash Size: ");
    SERIAL.print((uint32_t)flashSize);
    SERIAL.println("MB");
#else
    uint64_t cardSize = DEV.cardSize() / (1024 * 1024);
    SERIAL.print("SD Card Size: ");
    SERIAL.print((uint32_t)cardSize);
    SERIAL.println("MB");
#endif 


    listDir(DEV, "/", 0);
    createDir(DEV, "/mydir");
    listDir(DEV, "/", 0);
    removeDir(DEV, "/mydir");
    listDir(DEV, "/", 2);
    writeFile(DEV, "/hello.txt", "Hello ");
    appendFile(DEV, "/hello.txt", "World!\n");
    readFile(DEV, "/hello.txt");
    deleteFile(DEV, "/foo.txt");
    renameFile(DEV, "/hello.txt", "/foo.txt");
    readFile(DEV, "/foo.txt");
    testFileIO(DEV, "/foo.txt");
    uint32_t totalBytes = DEV.totalBytes();
    SERIAL.print("Total space: ");
    SERIAL.print(totalBytes / (1024 * 1024));
    SERIAL.println("MB");
    uint32_t usedBytes = DEV.usedBytes();
    SERIAL.print("Used space: ");
    SERIAL.print(usedBytes / (1024 * 1024));
    SERIAL.println("MB");
}

void loop() {
    
}