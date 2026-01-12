# Seeed-Arduino-FS  [![Build Status](https://travis-ci.com/Seeed-Studio/Seeed_Arduino_FS.svg?branch=master)](https://travis-ci.com/Seeed-Studio/Seeed_Arduino_FS)

## Introduction

A lightweight port of FatFs for Arduino. This library contains a routine to drive an SD card via SPI.The file system part is generic, which means you can easily port it to other types of memory, such as QSPI flash, emmc, etc.

An example is included in [Seeed_Arduino_SFUD](https://github.com/Seeed-Studio/Seeed_Arduino_SFUD)

## Usage

This library uses a single recommended entrypoint per backend:
- SPI SD: `seeedfs::SdSpiFatFs`
- SDMMC (WIO_LITE_AI): `seeedfs::SdmmcFatFs`

### SPI SD (most boards)

```c++
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

static seeedfs::SdSpiFatFs sd;

void setup() {
    LOG.begin(115200);
    while (!LOG) {}

    seeedfs::SdSpiCfg cfg;
    cfg.csPin = SDCARD_SS_PIN;
    cfg.spi = &SDCARD_SPI;
    cfg.spiHz = SDCARD_SPI_HZ;

    if (!sd.begin(cfg)) {
        LOG.print("SD init failed: ");
        LOG.println(sd.lastErrorString());
        return;
    }

    seeedfs::FS &fs = sd.fs();

    seeedfs::File f = fs.open("/hello.txt", "w");
    if (!f) {
        LOG.println("open /hello.txt failed");
        return;
    }
    f.println("hello 1, 2, 3.");
    f.close();

    seeedfs::File r = fs.open("/hello.txt", "r");
    while (r && r.available()) {
        LOG.write(r.read());
    }
    r.close();
}

void loop() {}
```

### SDMMC (WIO_LITE_AI)

```c++
#include <Seeed_Arduino_FS.h>

static seeedfs::SdmmcFatFs sd;

void setup() {
    Serial.begin(115200);
    while (!Serial) {}

    if (!sd.begin()) {
        Serial.print("SDMMC init failed: ");
        Serial.println(sd.lastErrorString());
        return;
    }

    seeedfs::FS &fs = sd.fs();
    // ... use fs.open/openDir/mkdir/etc
}

void loop() {}
```

## API Reference

- `seeedfs::SdSpiFatFs::begin(cfg)`: init SPI SD + start FATFS
- `seeedfs::SdmmcFatFs::begin()`: init SDMMC + start FATFS (WIO_LITE_AI)
- `open/openDir/mkdir/...`: same as `seeedfs::FS`

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

