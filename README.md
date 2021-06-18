# Seeed-Arduino-FS  [![Build Status](https://travis-ci.com/Seeed-Studio/Seeed_Arduino_FS.svg?branch=master)](https://travis-ci.com/Seeed-Studio/Seeed_Arduino_FS)

## Introduction

A lightweight port of FatFs for Arduino. This library contains a routine to drive an SD card via SPI.The file system part is generic, which means you can easily port it to other types of memory, such as QSPI flash, emmc, etc.

An example is included in [Seeed_Arduino_SFUD](https://github.com/Seeed-Studio/Seeed_Arduino_SFUD)

## Usage

**this code has been tested at wio terminal.**

```c++

#include <Seeed_Arduino_FS.h>

#define LOG Serial
#define DEV SD

#ifdef _SAMD21_
#define SDCARD_SS_PIN 1
#define SDCARD_SPI SPI
#endif 


void setup() {
    LOG.begin(115200);
    pinMode(5, OUTPUT);
    digitalWrite(5, HIGH);
    while (!LOG) {};
    while (!DEV.begin(SDCARD_SS_PIN,SDCARD_SPI,4000000UL)) {
        LOG.println("Card Mount Failed");
        return;
    }

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

