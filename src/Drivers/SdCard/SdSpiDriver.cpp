#include "Drivers/SdCard/SdSpiDriver.h"

#include "Utils/Errors.h"

#include <string.h>

namespace seeedfs
{

    SdSpiDriver::SdSpiDriver()
        : _csPin(0), _spi(nullptr), _spiHz(0), _hal(nullptr), _initialized(false), _type(SdCardType::None), _sectors(0)
    {
    }

    SdSpiDriver::~SdSpiDriver()
    {
        end();
    }

    void SdSpiDriver::configure(uint8_t csPin, SPIClass &spi, uint32_t spiHz)
    {
        _csPin = csPin;
        _spi = &spi;
        _spiHz = spiHz;
    }

    bool SdSpiDriver::begin()
    {
        if (_initialized)
        {
            return true;
        }

        if (_spi == nullptr || _spiHz == 0)
        {
            seeedfs_reportError(SEEED_FS_ERR_SD_INVALID_SPI);
            return false;
        }

        _hal = new SdCardHal(_csPin, *_spi, (int)_spiHz);
        if (!_hal->init())
        {
            // SdCardHal reports a more specific error; fallback to generic init failure.
            if (seeedfs_lastError() == SEEED_FS_OK)
            {
                seeedfs_reportError(SEEED_FS_ERR_SD_INIT_FAILED);
            }
            delete _hal;
            _hal = nullptr;
            return false;
        }

        _type = _hal->getType();
        _sectors = _hal->getSectors();
        _initialized = true;
        return true;
    }

    void SdSpiDriver::end()
    {
        if (_hal)
        {
            _hal->uninit();
            delete _hal;
            _hal = nullptr;
        }
        _initialized = false;
        _type = SdCardType::None;
        _sectors = 0;
    }

    bool SdSpiDriver::isReady() const
    {
        return _initialized;
    }

    uint32_t SdSpiDriver::sectorSize() const
    {
        return 512;
    }

    uint32_t SdSpiDriver::sectorCount() const
    {
        return _sectors;
    }

    uint64_t SdSpiDriver::size() const
    {
        return (uint64_t)_sectors * sectorSize();
    }

    const char *SdSpiDriver::type() const
    {
        return "SD";
    }

    bool SdSpiDriver::erase(uint32_t /*sector*/, size_t /*count*/)
    {
        return true;
    }

    SdCardType SdSpiDriver::cardType() const
    {
        return _type;
    }

    bool SdSpiDriver::readSectorsWithRetry(uint32_t sector, uint8_t *buffer, uint32_t count)
    {
        if (!_initialized || _hal == nullptr)
        {
            return false;
        }
        if (count == 0)
        {
            return true;
        }

        static const uint8_t kRetries = 3;
        for (uint8_t attempt = 0; attempt < kRetries; attempt++)
        {
            if (_hal->readSectors(sector, buffer, count))
            {
                return true;
            }
        }
        return false;
    }

    bool SdSpiDriver::writeSectorsWithRetry(uint32_t sector, const uint8_t *buffer, uint32_t count)
    {
        if (!_initialized || _hal == nullptr)
        {
            return false;
        }
        if (count == 0)
        {
            return true;
        }

        static const uint8_t kRetries = 3;
        for (uint8_t attempt = 0; attempt < kRetries; attempt++)
        {
            if (_hal->writeSectors(sector, buffer, count))
            {
                return true;
            }
        }
        return false;
    }

    bool SdSpiDriver::read(uint32_t sector, uint32_t offset, void *buffer, size_t size)
    {
        if (!_initialized || buffer == nullptr)
        {
            return false;
        }
        if (size == 0)
        {
            return true;
        }

        const uint32_t ss = sectorSize();
        if (ss == 0 || offset >= ss || (offset + size) > ss)
        {
            return false;
        }

        if (offset == 0 && size == ss)
        {
            return readSectorsWithRetry(sector, (uint8_t *)buffer, 1);
        }

        uint8_t tmp[512];
        if (ss != sizeof(tmp))
        {
            return false;
        }
        if (!readSectorsWithRetry(sector, tmp, 1))
        {
            return false;
        }
        memcpy((uint8_t *)buffer, tmp + offset, size);
        return true;
    }

    bool SdSpiDriver::write(uint32_t sector, uint32_t offset, const void *buffer, size_t size)
    {
        if (!_initialized || buffer == nullptr)
        {
            return false;
        }
        if (size == 0)
        {
            return true;
        }

        const uint32_t ss = sectorSize();
        if (ss == 0 || offset >= ss || (offset + size) > ss)
        {
            return false;
        }

        if (offset == 0 && size == ss)
        {
            return writeSectorsWithRetry(sector, (const uint8_t *)buffer, 1);
        }

        uint8_t tmp[512];
        if (ss != sizeof(tmp))
        {
            return false;
        }
        if (!readSectorsWithRetry(sector, tmp, 1))
        {
            return false;
        }
        memcpy(tmp + offset, (const uint8_t *)buffer, size);
        return writeSectorsWithRetry(sector, tmp, 1);
    }

} // namespace seeedfs
