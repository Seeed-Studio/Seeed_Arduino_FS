#include "SDMMC/Seeed_SDMMC.h"
#include <Seeed_FS.h>
#include "Arduino.h"

namespace fs {

    boolean SDMMCFS::begin(uint8_t ssPin, int hz) {
        // _pdrv = sdcard_init(ssPin, &spi, hz);
        // spi.begin();
        // FRESULT status;
        // _drv[0] = _T('0' + _pdrv);
        // status = f_mount(&rootSD, _drv, 1);
        // if (status != FR_OK) {
        //     return false;
        // } else {
        //     return true;
        // }
        return true;
    }

    void SDMMCFS::end() {
        // if (_pdrv != 0xFF) {
        //     f_mount(NULL, _drv, 1);
        //     sdcard_uninit(_pdrv);
        //     _pdrv = 0xFF;
        // }
    }

    uint64_t SDMMCFS::cardSize() {
        // if (_pdrv == 0xFF) {
        //     return 0;
        // }
        // size_t sectors = sdcard_num_sectors(_pdrv);
        // size_t sectorSize = sdcard_sector_size(_pdrv);
        // return (uint64_t)sectors * sectorSize;
    }


    uint64_t SDMMCFS::totalBytes() {
        // FATFS* fsinfo;
        // DWORD fre_clust;
        // if (f_getfree(_drv, &fre_clust, &fsinfo) != 0) {
        //     return 0;
        // }
        // uint64_t size = ((uint64_t)(fsinfo->csize)) * (fsinfo->n_fatent - 2)
        //                 #if _MAX_SS != 512
        //                 * (fsinfo->ssize);
        //                 #else
        //                 * 512;
        //                 #endif
        // return size;
        return 0;
    }

    uint64_t SDMMCFS::usedBytes() {
        // FATFS* fsinfo;
        // DWORD fre_clust;
        // if (f_getfree(_drv, &fre_clust, &fsinfo) != 0) {
        //     return 0;
        // }
        // uint64_t size = ((uint64_t)(fsinfo->csize)) * ((fsinfo->n_fatent - 2) - (fsinfo->free_clst))
        //                 #if _MAX_SS != 512
        //                 * (fsinfo->ssize);
        //                 #else
        //                 * 512;
        //                 #endif
        // return size;
        return 0;
    }
};
