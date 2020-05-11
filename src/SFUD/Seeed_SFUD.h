/*

    SD - a slightly more friendly wrapper for sdfatlib

    This library aims to expose a subset of SD card functionality
    in the form of a higher level "wrapper" object.

    License: GNU General Public License V3
          (Because sdfatlib is licensed with this.)

    (C) Copyright 2010 SparkFun Electronics

*/

#ifndef __SEEED_SFUD_H__
#define __SEEED_SFUD_H__

#include <sfud.h>
#include <Arduino.h>
#include <Seeed_FS.h>
extern "C" {
#include "../fatfs/diskio.h"
#include "../fatfs/ffconf.h"
#include "../fatfs/ff.h"
}
typedef enum {
    FLASH_NONE,
    FLASH_SPI
} sfud_type_t;

#define SECTORSIZE 512

typedef struct {
    uint8_t ssPin;
    int16_t sector_size;
    sfud_type_t type;
    unsigned long sectors;
    int status;
} ardu_sfud_t;

static ardu_sfud_t* s_sfuds[_VOLUMES] = { NULL };
namespace fs {

    class SFUDFS : public FS {
      private:
        uint8_t _pdrv;
        TCHAR _drv[2] = {_T('0' + _pdrv), _T(':')};
        boolean init();
      public:
        SFUDFS() {}
        ~SFUDFS() {}
        // This needs to be called to set up the connection to the SD card
        // before other methods are used.
        boolean begin(uint8_t ssPin, SPIClass& spi, int hz); //SPI
        boolean begin(int hz); //QSPI
        //call this when a card is removed. It will allow you to insert and initialise a new card.
        boolean end();

        sfud_type_t flashType();
        uint64_t flashSize();
        uint64_t totalBytes();
        uint64_t usedBytes();
    };
    
    extern SFUDFS SPIFLASH;
};

using namespace fs;
typedef fs::File        SPIFLASHFile;
typedef fs::SFUDFS      SPIFLASHFileSystemClass;
#define SPIFLASHFileSystemClass    SPIFLASH


#endif
