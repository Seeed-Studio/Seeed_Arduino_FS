/*
    a wrapper for SDflash.

    Created by Hongtai.liu 13 July 2019

*/

#include "Seeed_SFUD.h"

DSTATUS flash_disk_initialize(BYTE pdrv);
DSTATUS flash_disk_status(BYTE pdrv);
DRESULT flash_disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count);
DRESULT flash_disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count);
DRESULT flash_disk_ioctl(BYTE pdrv, BYTE cmd, void* buff);

namespace fs {

    boolean SFUDFS::begin(uint8_t ssPin, SPIClass& spi, int hz) {
        #define SPIDEV *spi
        chipSelectPin = ssPin;
        SPIclock = hz;
        return init();
    }
    boolean SFUDFS::begin(int hz){
        QSPIclock = hz;
        return init();
    }
    boolean SFUDFS::init(void){
        _pdrv = 0xff;
        if (ff_diskio_get_drive(&_pdrv) != 0 || _pdrv == 0xFF) {
            return false;
        }      
        ardu_sfud_t* flash_t = (ardu_sfud_t*)malloc(sizeof(ardu_sfud_t));
        flash_t->type = FLASH_NONE;
        flash_t->status = STA_NOINIT;
        flash_t->sector_size = SECTORSIZE;    
        s_sfuds[_pdrv] = flash_t; 
        static const ff_diskio_impl_t flash_impl = {
            .init = &flash_disk_initialize,
            .status = &flash_disk_status,
            .read = &flash_disk_read,
            .write = &flash_disk_write,
            .ioctl = &flash_disk_ioctl
        };
        ff_diskio_register(_pdrv, &flash_impl);
        FRESULT status;
        _drv[0] = _T('0' + _pdrv);
        status = f_mount(&rootFLASH, _drv, 1);
        SEEED_FS_DEBUG("The available drive number : %d",_pdrv);
        SEEED_FS_DEBUG("The status of f_mount : %d",status);
        SEEED_FS_DEBUG("more information about the status , you can view the FRESULT enum");
        if (status == FR_NO_FILESYSTEM){
            BYTE work[512]; /* Work area (larger is better for processing time) */
            FRESULT ret;
            ret = f_mkfs(_drv, FM_FAT, 0, work, sizeof(work));
            SEEED_FS_DEBUG("The status of f_mkfs : %d",ret);
            SEEED_FS_DEBUG("more information about the status , you can view the FRESULT enum");            
            status = f_mount(&rootFLASH,_drv, 1);
        }
        if (status != FR_OK) {
            return false;
        } else {
            return true;
        }
    }
    boolean SFUDFS::end() {
        if (_pdrv != 0xFF) {
            
            f_mount(NULL, _drv, 1);
            ardu_sfud_t* flash_t = s_sfuds[_pdrv];
            if (_pdrv >= _VOLUMES || flash_t == NULL) {
                return false;
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
            return false;
        }
        ardu_sfud_t* flash_t = s_sfuds[_pdrv];
        if (_pdrv >= _VOLUMES || flash_t == NULL) {
            return false;
        }
        SEEED_FS_DEBUG("The sectors: %d , The sector size: %d",flash_t->sectors,flash_t->sector_size);
        return flash_t->sectors * flash_t->sector_size;
    }
    uint64_t SFUDFS::totalBytes() {
        FATFS* fsinfo;
        FRESULT ret = FR_OK;
        DWORD free_clust;
        
        ret = f_getfree(_drv, &free_clust, &fsinfo);
        SEEED_FS_DEBUG("The status of f_getfree : %d",ret);
        SEEED_FS_DEBUG("more information about the status , you can view the FRESULT enum");
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
        DWORD free_clust;
        FRESULT ret = FR_OK;
        ret = f_getfree(_drv, &free_clust, &fsinfo);
        SEEED_FS_DEBUG("The status of f_getfree : %d",ret);
        SEEED_FS_DEBUG("more information about the status , you can view the FRESULT enum");
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
const sfud_flash* flash = (sfud_flash*)malloc(sizeof(sfud_flash));

DSTATUS flash_disk_initialize(uint8_t pdrv){
    ardu_sfud_t* flash_t = s_sfuds[pdrv];
    if (!(flash_t->status & STA_NOINIT)) {
        return flash_t->status;
    }
    if(sfud_init()==SFUD_SUCCESS){
        flash_t->status &= ~STA_NOINIT;
        flash = sfud_get_device_table() + 0;
        flash_t->sectors = flash->chip.capacity / flash_t->sector_size;
#ifdef SFUD_USING_QSPI
        sfud_qspi_fast_read_enable(sfud_get_device(0), 2);
#endif
        flash_t->type = FLASH_SPI;
        return flash_t->status;
    }
    flash_t->type = FLASH_NONE;
    return flash_t->status;
}
DSTATUS flash_disk_status(uint8_t pdrv) {
    ardu_sfud_t* flash_t = s_sfuds[pdrv];
    return flash_t->status;
}
DRESULT flash_disk_read(uint8_t pdrv, uint8_t* buffer, DWORD sector, UINT count) {
    ardu_sfud_t* flash_t = s_sfuds[pdrv];
    if (flash_t->status & STA_NOINIT) {
        return RES_NOTRDY;
    }
    DRESULT res = RES_OK;
    sector = sector * flash_t->sector_size;
    count = count * flash_t->sector_size;
    res = sfud_read(flash,sector,count,buffer) ? RES_ERROR : RES_OK;
    return res;
}
DRESULT flash_disk_write(uint8_t pdrv, const uint8_t* buffer, DWORD sector, UINT count) {
    ardu_sfud_t* flash_t = s_sfuds[pdrv];
    if (flash_t->status & STA_NOINIT) {
        return RES_NOTRDY;
    }
    DRESULT res = RES_OK;    
    sector = sector * flash_t->sector_size;
    count = count * flash_t->sector_size;
    res = sfud_erase_write(flash,sector,count,buffer) ? RES_ERROR : RES_OK;
    return res;
}
DRESULT flash_disk_ioctl(uint8_t pdrv, uint8_t cmd, void* buff) {
    switch (cmd) {
        case CTRL_SYNC: 
            return RES_OK;
        case GET_SECTOR_COUNT:
            *((unsigned long*) buff) = s_sfuds[pdrv]->sectors;
            return RES_OK;
        case GET_SECTOR_SIZE:
            *((WORD*) buff) = s_sfuds[pdrv]->sector_size;
            return RES_OK;
        case GET_BLOCK_SIZE:
            *((uint32_t*)buff) = 1;
            return RES_OK;
    }
    return RES_PARERR;
}