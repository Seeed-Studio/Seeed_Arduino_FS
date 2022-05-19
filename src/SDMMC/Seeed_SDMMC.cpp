#ifdef WIO_LITE_AI
#include "Seeed_SDMMC.h"
#include "Seeed_SDMMC_Interface.h"
#include <Seeed_FS.h>
#include "Arduino.h"

namespace fs{

    boolean SDMMCFS::begin()
    {
        _pdrv = 0xff;
        if (ff_diskio_get_drive(&_pdrv) != 0 || _pdrv == 0xFF) {
            return false;
        }    
        static const ff_diskio_impl_t flash_impl = {
            .init   = &SD_initialize,
            .status = &SD_status,
            .read   = &SD_read,
            .write  = &SD_write,
            .ioctl  = &SD_ioctl
        };
        if(MX_SDMMC1_SD_Init() != HAL_OK)
        {
            return false; 
        }
        ff_diskio_register(_pdrv, &flash_impl);    
        
        FRESULT status;
        _drv[0] = _T('0' + _pdrv);
        status = f_mount(&root, _drv, 0);
        if (status == FR_OK) {
            return true;
        } else {
            return false;
        }
        return true;
    }

    void SDMMCFS::end()
    {
        if (_pdrv != 0xFF) {
            f_mount(NULL, _drv, 1);
            HAL_SD_MspDeInit(&hsd1);
            _pdrv = 0xFF;
        }
    }

    uint8_t SDMMCFS::getPhysicalDriveNumber() {
        return _pdrv;
    }

    String SDMMCFS::getDriveLetter() {
        if (_pdrv == 0xFF) {
            // Card not initialized
            return "";
        }
        return String(_pdrv) + ":";
    }


    int SDMMCFS::cardType()
    {
        if (_pdrv == 0xFF) {
            return CARD_NONE;
        }
        return (hsd1.SdCard.CardType + 1);
    }

    uint64_t SDMMCFS::cardSize()
    {
        if (_pdrv == 0xFF) {
            return 0;
        }
        return (uint64_t)((uint64_t)hsd1.SdCard.BlockNbr * (uint64_t)hsd1.SdCard.BlockSize);
    }

    uint64_t SDMMCFS::totalBytes()
    {
        FATFS* fsinfo;
        DWORD fre_clust;
        if (f_getfree(_drv, &fre_clust, &fsinfo) != 0) {
            return 0;
        }
        uint64_t size = ((uint64_t)(fsinfo->csize)) * (fsinfo->n_fatent - 2)
                        #if _MAX_SS != 512
                        * (fsinfo->ssize);
                        #else
                        * 512;
                        #endif
        return size;
    }

    uint64_t SDMMCFS::usedBytes()
    {
        FATFS* fsinfo;
        DWORD fre_clust;
        if (f_getfree(_drv, &fre_clust, &fsinfo) != 0) {
            return 0;
        }
        uint64_t size = ((uint64_t)(fsinfo->csize)) * ((uint64_t)(fsinfo->n_fatent - 2) - (uint64_t)(fsinfo->free_clst))
                        #if _MAX_SS != 512
                        * (fsinfo->ssize);
                        #else
                        * 512;
                        #endif
        return size;
    }
    SDMMCFS SDMMC;
}
#endif