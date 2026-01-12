#pragma once

#include <Arduino.h>
#include <SPI.h>

#include "Storage/StorageImpl.h"
#include "Drivers/SdCard/SdCardHal.h"

namespace seeedfs
{

    class SdSpiDriver : public StorageImpl
    {
    public:
        SdSpiDriver();
        ~SdSpiDriver() override;

        void configure(uint8_t csPin, SPIClass &spi, uint32_t spiHz);

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
        bool readSectorsWithRetry(uint32_t sector, uint8_t *buffer, uint32_t count);
        bool writeSectorsWithRetry(uint32_t sector, const uint8_t *buffer, uint32_t count);

        uint8_t _csPin;
        SPIClass *_spi;
        uint32_t _spiHz;

        SdCardHal *_hal;
        bool _initialized;
        SdCardType _type;
        uint32_t _sectors;
    };

} // namespace seeedfs
