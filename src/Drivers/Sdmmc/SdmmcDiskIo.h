#pragma once

#ifdef WIO_LITE_AI

#include <Arduino.h>

#include "Drivers/Sdmmc/SdmmcHalStm32.h"

// FatFs diskio types
#include "FileSystem/FATFS/fatfs/diskio.h"

uint8_t MX_SDMMC1_SD_Init(void);
DSTATUS SD_initialize(BYTE pdrv);
DSTATUS SD_status(BYTE pdrv);
DRESULT SD_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count);
DRESULT SD_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count);
DRESULT SD_ioctl(BYTE pdrv, BYTE cmd, void *buff);

#endif // WIO_LITE_AI
