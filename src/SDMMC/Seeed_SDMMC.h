#ifndef __SEEED_SDMMC_H__
#define __SEEED_SDMMC_H__

#include <Arduino.h>

#ifdef WIO_LITE_AI

#include <SD/Seeed_sdcard_hal.h>
#include <Seeed_FS.h>

namespace fs {

    class SDMMCFS : public FS {
      private:
        uint8_t _pdrv;
        TCHAR _drv[2] = {_T('0' + _pdrv), _T(':')};
      public:
        SDMMCFS() {}
        ~SDMMCFS() {}
        // This needs to be called to set up the connection to the SD card
        // before other methods are used.
        boolean begin();

        //call this when a card is removed. It will allow you to insert and initialise a new card.
        void end();

        uint8_t getPhysicalDriveNumber();
        String getDriveLetter();

        int cardType();
        uint64_t cardSize();
        uint64_t totalBytes();
        uint64_t usedBytes();

    };
    extern SDMMCFS SDMMC;
};

#endif 

#endif 