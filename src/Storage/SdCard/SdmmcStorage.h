#ifndef __SEEED_SDMMC_H__
#define __SEEED_SDMMC_H__

#include <Arduino.h>

#ifdef WIO_LITE_AI

#include "Drivers/Sdmmc/SdmmcDriverStm32.h"

namespace seeedfs
{

  // New-architecture SDMMC storage (StorageImpl) for WIO_LITE_AI.
  // Use with FatFs:
  //   static seeedfs::SdmmcStorage sdmmc;
  //   static seeedfs::FatFs fatfs(&sdmmc);
  //   static seeedfs::FS DEV(&fatfs);
  class SdmmcStorage : public SdmmcDriverStm32
  {
  public:
    using SdmmcDriverStm32::SdmmcDriverStm32; // Inherit constructors
    ~SdmmcStorage() override = default;
  };
} // namespace seeedfs

#endif

#endif