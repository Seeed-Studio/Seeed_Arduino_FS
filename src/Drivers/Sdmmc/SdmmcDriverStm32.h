#pragma once

#include <stddef.h>
#include <stdint.h>

#include "Storage/StorageImpl.h"
#include "Drivers/SdCard/SdCardTypes.h"

#ifdef WIO_LITE_AI
#include "Drivers/Sdmmc/SdmmcHalStm32.h"
#endif

namespace seeedfs
{

    // SDMMC driver implementation for STM32H7 (WIO_LITE_AI).
    // This is an SD-card driver (media is still SD), using SDMMC peripheral.
    class SdmmcDriverStm32 : public StorageImpl
    {
    public:
        SdmmcDriverStm32();
        ~SdmmcDriverStm32() override;

        bool begin() override;
        void end() override;
        bool isReady() const override;

        uint32_t sectorSize() const override;
        uint32_t sectorCount() const override;

        // StorageImpl specific
        uint64_t size() const override;
        const char *type() const override;
        bool erase(uint32_t sector, size_t count) override;

        // SD specific
        SdCardType cardType() const;

        bool read(uint32_t sector, uint32_t offset, void *buffer, size_t size) override;
        bool write(uint32_t sector, uint32_t offset, const void *buffer, size_t size) override;

        bool sync() override { return true; }

    private:
        bool refreshCardInfo();

    private:
        bool _ready;
        uint32_t _sectorSize;
        uint32_t _sectorCount;
        SdCardType _type;
    };

} // namespace seeedfs
