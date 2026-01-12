#pragma once

#ifdef WIO_LITE_AI

#include "stm32h7xx_hal.h"

// Low-level init symbol provided by the WIO_LITE_AI core (or this library's SdmmcDiskIo).
uint8_t MX_SDMMC1_SD_Init(void);

// SDMMC handle (provided by core or SdmmcDiskIo).
extern SD_HandleTypeDef hsd1;

#endif // WIO_LITE_AI
