#ifndef __SEEED_SDMMC_INTERFACE_H_
#define __SEEED_SDMMC_INTERFACE_H_
#ifdef WIO_LITE_AI

#include <Arduino.h>
#include <Seeed_FS.h>
uint8_t MX_SDMMC1_SD_Init(void);
DSTATUS SD_initialize(BYTE pdrv);
DSTATUS SD_status(BYTE pdrv);
DRESULT SD_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count);
DRESULT SD_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count);
DRESULT SD_ioctl(BYTE pdrv, BYTE cmd, void* buff);

extern SD_HandleTypeDef hsd1;

#endif
#endif
