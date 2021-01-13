
#ifndef __SDMMC_H__
#define __SDMMC_H__
#ifdef ARDUINO_Seeeduino_H7AI
#include <Arduino.h>
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
        boolean begin(uint8_t ssPin = 11, int hz = 4000000);

        //call this when a card is removed. It will allow you to insert and initialise a new card.
        void end();
        uint64_t cardSize();
        uint64_t totalBytes();
        uint64_t usedBytes();
    };
};

#endif //ARDUINO_Seeeduino_H7AI

#endif //__SDMMC_H__
