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

#define LOG Serial

#ifndef SDCARD_SS_PIN
#define SDCARD_SS_PIN D2
#endif

#ifndef SDCARD_SPI
#define SDCARD_SPI SPI
#endif

#ifndef SDCARD_SPI_HZ
#define SDCARD_SPI_HZ 400000UL
#endif

// Seeed XIAO (ESP32 family) default SPI pins
#ifndef SDCARD_SCK_PIN
#define SDCARD_SCK_PIN  D8
#endif
#ifndef SDCARD_MISO_PIN
#define SDCARD_MISO_PIN D9
#endif
#ifndef SDCARD_MOSI_PIN
#define SDCARD_MOSI_PIN D10
#endif

#ifdef WIO_LITE_AI
static seeedfs::SdmmcFatFs g_fs;
#else
static seeedfs::SdSpiFatFs g_fs;
#endif

void setup() {
    LOG.begin(115200);
    pinMode(5, OUTPUT);
    digitalWrite(5, HIGH);
    while (!LOG) {};

#ifdef WIO_LITE_AI
    if (!g_fs.begin()) {
        LOG.print("SDMMC init failed: ");
        LOG.println(g_fs.lastErrorString());
        return;
    }
#else
    seeedfs::SdSpiCfg cfg;
    cfg.csPin = SDCARD_SS_PIN;
    cfg.spi = &SDCARD_SPI;
    cfg.spiHz = SDCARD_SPI_HZ;
#if defined(ARDUINO_ARCH_ESP32)
    cfg.sckPin = SDCARD_SCK_PIN;
    cfg.misoPin = SDCARD_MISO_PIN;
    cfg.mosiPin = SDCARD_MOSI_PIN;
#endif
    if (!g_fs.begin(cfg)) {
        LOG.print("SD init failed: ");
        LOG.println(g_fs.lastErrorString());
        return;
    }
#endif

    seeedfs::FS &DEV = g_fs.fs();
    LOG.println("initialization done.");

    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.

    seeedfs::File RootWrite = DEV.open("/hello.txt", "w");
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
    seeedfs::File RootRead= DEV.open("/hello.txt");
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


