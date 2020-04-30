/*
    a wrapper for SDflash.

    Created by Hongtai.liu 13 July 2019

*/

#include "Seeed_SFUD.h"

namespace fs {

    boolean SFUDFS::begin(uint8_t ssPin, SPIClass& spi, int hz) {
        // uint8_t pdrv = 0xFF;
        BYTE work[1024]; /* Work area (larger is better for processing time) */
        _pdrv = 0xff;
        if (ff_diskio_get_drive(&_pdrv) != 0 || _pdrv == 0xFF) {
            return false;
        }      
        ardu_sfud_t* flash_t = (ardu_sfud_t*)malloc(sizeof(ardu_sfud_t));
        flash_t->ssPin = ssPin;
        // chipSelectPin = flash_t->ssPin;
        flash_t->type = FLASH_NONE;
        flash_t->status |= STA_NOINIT;
        s_sfuds[_pdrv] = flash_t;
        static const ff_diskio_impl_t flash_impl = {
            .init = &disk_initialize,
            .status = &disk_status,
            .read = &disk_read,
            .write = &disk_write,
            .ioctl = &disk_ioctl            
        };
        ff_diskio_register(_pdrv, &flash_impl);
        FRESULT status;
        Serial.println(f_mkfs(_T("0:"), FM_ANY, 0, work, sizeof(work)));
        FATFS fs;
        status = f_mount(&root, _T("0:"), 1);
        Serial.println(status);
        if (status != FR_OK) {
            return false;
        } else {
            return true;
        }
    }
    boolean SFUDFS::end() {
        if (_pdrv != 0xFF) {
            f_mount(NULL, _T("0:"), 1);
            ardu_sfud_t* flash_t = s_sfuds[_pdrv];
            if (_pdrv >= _VOLUMES || flash_t == NULL) {
                return 0;
            }
            ff_diskio_register(_pdrv, NULL);
            flash_t->status |= STA_NOINIT;
            s_sfuds[_pdrv] = NULL;
            free(flash_t);
            _pdrv = 0xFF;
        }
    }

    sfud_type_t SFUDFS::flashType() {
        if (_pdrv == 0xFF) {
            return FLASH_NONE;
        }
        ardu_sfud_t* flash_t = s_sfuds[_pdrv];
        if (_pdrv >= _VOLUMES || flash_t == NULL) {
            return FLASH_NONE;
        }   
        return flash_t->type;
    }

    uint64_t SFUDFS::flashSize() {
        if (_pdrv == 0xFF) {
            return 0;
        }        
        ardu_sfud_t* flash_t = s_sfuds[_pdrv];
        if (_pdrv >= _VOLUMES || flash_t == NULL) {
            return 0;
        }
        return flash_t->sectors * SECTORSIZE;
    }

    uint64_t SFUDFS::totalBytes() {
        FATFS* fsinfo;
        DWORD fre_clust;
        if (f_getfree("0:", &fre_clust, &fsinfo) != 0) {
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

    uint64_t SFUDFS::usedBytes() {
        FATFS* fsinfo;
        DWORD fre_clust;
        if (f_getfree("0:", &fre_clust, &fsinfo) != 0) {
            return 0;
        }
        uint64_t size = ((uint64_t)(fsinfo->csize)) * ((fsinfo->n_fatent - 2) - (fsinfo->free_clst))
                        #if _MAX_SS != 512
                        * (fsinfo->ssize);
                        #else
                        * 512;
                        #endif
        return size;
    }
    SFUDFS SPIFLASH;
};
#define USESPIFLASH
#ifdef USESPIFLASH
const sfud_flash* flash = NULL;

DSTATUS disk_initialize(uint8_t pdrv){
    Serial.println('1');
    ardu_sfud_t* flash_t = s_sfuds[pdrv];
    if (!(flash_t->status & STA_NOINIT)) {
        return flash_t->status;
    }
    if(sfud_init()==SFUD_SUCCESS){
        flash_t->status &= ~STA_NOINIT;
        flash = sfud_get_device_table() + 0;
        flash_t->sectors = flash->chip.capacity / SECTORSIZE;
        flash_t->type = FLASH_SPI;
        return flash_t->status;
    }
    flash_t->type = FLASH_NONE;
    return flash_t->status;
}
DSTATUS disk_status(uint8_t pdrv) {
    Serial.println('2');
    return s_sfuds[pdrv]->status;
}
DRESULT disk_read(uint8_t pdrv, uint8_t* buffer, DWORD sector, UINT count) {
    ardu_sfud_t* flash_t = s_sfuds[pdrv];
    Serial.println('3');
    if (flash_t->status & STA_NOINIT) {
        return RES_NOTRDY;
    }
    DRESULT res = RES_OK;
    // for(;count>0;count--){
    //     if (sfud_read(flash,sector*SECTORSIZE,SECTORSIZE,buffer)){
    //         return RES_ERROR;
    //     }
    //     sector++;
    //     buffer+=SECTORSIZE;
    // }
    sector = sector * SECTORSIZE;
    count = count * SECTORSIZE;
    res = sfud_read(flash,sector,count,buffer) ? RES_ERROR : RES_OK;
    return res;
}
DRESULT disk_write(uint8_t pdrv, const uint8_t* buffer, DWORD sector, UINT count) {
    ardu_sfud_t* flash_t = s_sfuds[pdrv];
    Serial.println('4');
    if (flash_t->status & STA_NOINIT) {
        return RES_NOTRDY;
    }
    DRESULT res = RES_OK;    
    sector = SECTORSIZE*sector;
    count = SECTORSIZE*count;
    res = sfud_erase_write(flash,sector,count,buffer) ? RES_ERROR : RES_OK;
    return res;
}
DRESULT disk_ioctl(uint8_t pdrv, uint8_t cmd, void* buff) {
    Serial.println('5');
    switch (cmd) {
        case CTRL_SYNC: 
            return RES_OK;
        case GET_SECTOR_COUNT:
            *((unsigned long*) buff) = s_sfuds[pdrv]->sectors;
            return RES_OK;
        case GET_SECTOR_SIZE:
            *((WORD*) buff) = SECTORSIZE;
            return RES_OK;
        case GET_BLOCK_SIZE:
            *((uint32_t*)buff) = 1;
            return RES_OK;
    }
    return RES_PARERR;        
}
#endif