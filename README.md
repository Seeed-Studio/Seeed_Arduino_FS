# Seeed-Arduino-FS  [![Build Status](https://travis-ci.com/Seeed-Studio/Seeed_Arduino_FS.svg?branch=master)](https://travis-ci.com/Seeed-Studio/Seeed_Arduino_FS)

## Introduction

An Arduino library for the file operation.there exist FAT file systems in this lib to install the FS at SD card and SPI flash,meanwhile that use [Seeed SFUD](https://github.com/Seeed-Studio/Seeed_Arduino_SFUD) to support a lot of type of SPI flash.the FAT type can be support by using this lib.the more information we can get by accessing our [wiki](https://wiki.seeedstudio.com/Wio-Terminal-FS-ReadWrite/)

## Usage

**this code has been tested at wio terminal.**

```c++

#include <Seeed_FS.h>
#define SERIAL Serial
#ifdef USESPIFLASH    // if you want to use flash FS . default mode is spi mode
#define DEV SPIFLASH
#include "SFUD/Seeed_SFUD.h"
#else                 // if you want to use SD FS
#define DEV SD
#include "SD/Seeed_SD.h"
#endif

void setup() {
    // Open SERIAL communications and wait for port to open:
    SERIAL.begin(115200);
    while (!SERIAL) {
        ; // wait for SERIAL port to connect. Needed for native USB port only
    }

    //init SPI flash or SPI SD card determined by USESPIFLASH
    SERIAL.print("Initializing storage device...");
    while (!DEV.begin(SDCARD_SS_PIN,SDCARD_SPI,4000000UL)) {
        SERIAL.println("Card Mount Failed");
        return;
    }
    //you can use DEV.begin(104000000UL) to init device if you want to use QSPI flash.

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

```

## API Reference

- boolean begin(uint8_t ssPin, SPIClass& sp, int hz) : config the SPI to control storage device

```c++
DEV.begin(SDCARD_SS_PIN,SDCARD_SPI,4000000UL)
// DEV.begin(104000000UL) //use qspi flash
```

- sdcard_type_t  cardType() : get SD card type 

**Note** : only work with SD card

```c++
    uint8_t cardType = DEV.cardType();
    if (cardType == CARD_NONE) {
        SERIAL.println("No SD card attached");
        return;
    }
```

- sfud_type_t   flashType() : get flash type

**Note** : only work with flash

```c++
    uint8_t flashType = DEV.flashType();
    if (flashType == FLASH_NONE) {
        SERIAL.println("No flash attached");
        return;
    }
```

- uint64_t cardSize(): get SD card size

**Note** : only work with SD card

```c++
    uint64_t cardSize = DEV.cardSize() / (1024 * 1024);
    SERIAL.print("SD Card Size: ");
    SERIAL.print((uint32_t)cardSize);
    SERIAL.println("MB");
```

- uint64_t    flashSize() : get flash size

**Note** : only work with flash

```c++
    uint32_t flashSize = DEV.flashSize() / (1024 * 1024);
    SERIAL.print("flash Size: ");
    SERIAL.print((uint32_t)flashSize);
    SERIAL.println("MB");
```

- uint64_t totalBytes(): return total Bytes of storage device

```c++
    uint32_t totalBytes = DEV.totalBytes();
    SERIAL.print("Total space: ");
    SERIAL.print(totalBytes / (1024 * 1024));
    SERIAL.println("MB");
```

- uint64_t usedBytes(): return used Bytes of storage device

```c++
    uint32_t usedBytes = DEV.usedBytes();
    SERIAL.print("Used space: ");
    SERIAL.print(usedBytes / (1024 * 1024));
    SERIAL.println("MB");
```

----

This software is written by seeed studio<br>
and is licensed under [The MIT License](http://opensource.org/licenses/mit-license.php). Check License.txt for more information.<br>

Contributing to this software is warmly welcomed. You can do this basically by<br>
[forking](https://help.github.com/articles/fork-a-repo), committing modifications and then [pulling requests](https://help.github.com/articles/using-pull-requests) (follow the links above<br>
for operating guide). Adding change log and your contact into file header is encouraged.<br>
Thanks for your contribution.

Seeed Studio is an open hardware facilitation company based in Shenzhen, China. <br>
Benefiting from local manufacture power and convenient global logistic system, <br>
we integrate resources to serve new era of innovation. Seeed also works with <br>
global distributors and partners to push open hardware movement.<br>


[![Analytics](https://ga-beacon.appspot.com/UA-46589105-3/Grove_LED_Bar)](https://github.com/igrigorik/ga-beacon)

