#include "Drivers/Sdmmc/SdmmcDriverStm32.h"

#include "Utils/Errors.h"

#ifdef WIO_LITE_AI

#include <string.h>

#ifndef SD_WRITE_TIMEOUT
#define SD_WRITE_TIMEOUT 100U
#endif

#ifndef SD_READ_TIMEOUT
#define SD_READ_TIMEOUT 100U
#endif

namespace seeedfs
{

    static const uint32_t kSectorSizeDefault = 512;

    SdmmcDriverStm32::SdmmcDriverStm32() : _ready(false), _sectorSize(kSectorSizeDefault), _sectorCount(0), _type(SdCardType::Unknown) {}

    SdmmcDriverStm32::~SdmmcDriverStm32()
    {
        end();
    }

    bool SdmmcDriverStm32::begin()
    {
        _ready = false;
        _sectorSize = kSectorSizeDefault;
        _sectorCount = 0;
        _type = SdCardType::Unknown;

        if (MX_SDMMC1_SD_Init() != HAL_OK)
        {
            seeedfs_reportError(SEEED_FS_ERR_STORAGE_BEGIN_FAILED);
            return false;
        }

        if (!refreshCardInfo())
        {
            // refreshCardInfo currently validates sector size/count; use a generic storage error.
            seeedfs_reportError(SEEED_FS_ERR_STORAGE_BEGIN_FAILED);
            return false;
        }

        _ready = true;
        return true;
    }

    void SdmmcDriverStm32::end()
    {
        _ready = false;
        _sectorCount = 0;
        _sectorSize = kSectorSizeDefault;
        _type = SdCardType::Unknown;

        // De-init low-level peripheral (safe to call even if not inited).
        HAL_SD_MspDeInit(&hsd1);
    }

    bool SdmmcDriverStm32::isReady() const
    {
        if (!_ready)
        {
            return false;
        }
        return (HAL_SD_GetCardState(const_cast<SD_HandleTypeDef *>(&hsd1)) == HAL_SD_CARD_TRANSFER);
    }

    uint32_t SdmmcDriverStm32::sectorSize() const
    {
        return _sectorSize ? _sectorSize : kSectorSizeDefault;
    }

    uint32_t SdmmcDriverStm32::sectorCount() const
    {
        return _sectorCount;
    }

    uint64_t SdmmcDriverStm32::size() const
    {
        return (uint64_t)_sectorCount * sectorSize();
    }

    const char *SdmmcDriverStm32::type() const
    {
        return "SDMMC";
    }

    bool SdmmcDriverStm32::erase(uint32_t /*sector*/, size_t /*count*/)
    {
        return true;
    }

    SdCardType SdmmcDriverStm32::cardType() const
    {
        return _type;
    }

    bool SdmmcDriverStm32::refreshCardInfo()
    {
        HAL_SD_CardInfoTypeDef info;
        if (HAL_SD_GetCardInfo(&hsd1, &info) != HAL_OK)
        {
            return false;
        }

        _sectorSize = info.LogBlockSize;
        _sectorCount = info.LogBlockNbr;

        // Map HAL card type to our SdCardType.
        // Values come from STM32 HAL (stm32h7xx_hal_sd.h).
        switch (info.CardType)
        {
        case CARD_SDSC:
            _type = SdCardType::Sd;
            break;
        case CARD_SDHC_SDXC:
            _type = SdCardType::Sdhc;
            break;
        default:
            _type = SdCardType::Unknown;
            break;
        }

        // We currently only support 512-byte sectors through the stack.
        return (_sectorSize == kSectorSizeDefault) && (_sectorCount != 0);
    }

    bool SdmmcDriverStm32::read(uint32_t sector, uint32_t offset, void *buffer, size_t size)
    {
        if (!isReady() || buffer == nullptr)
        {
            return false;
        }

        if (size == 0)
        {
            return true;
        }

        const uint32_t ss = sectorSize();
        if (ss != kSectorSizeDefault || offset >= ss || (offset + size) > ss)
        {
            return false;
        }

        // Fast path: full sector read.
        if (offset == 0 && size == ss)
        {
            const bool aligned = ((((uintptr_t)buffer) & 0x3U) == 0);
            if (aligned)
            {
                if (HAL_SD_ReadBlocks(&hsd1, (uint8_t *)buffer, sector, 1, SD_READ_TIMEOUT) != HAL_OK)
                {
                    return false;
                }
                while (HAL_SD_GetCardState(&hsd1) != HAL_SD_CARD_TRANSFER)
                {
                }
                return true;
            }

            alignas(4) uint8_t tmp[512];
            if (HAL_SD_ReadBlocks(&hsd1, tmp, sector, 1, SD_READ_TIMEOUT) != HAL_OK)
            {
                return false;
            }
            while (HAL_SD_GetCardState(&hsd1) != HAL_SD_CARD_TRANSFER)
            {
            }
            memcpy((uint8_t *)buffer, tmp, ss);
            return true;
        }

        // Partial sector: read-modify-copy.
        alignas(4) uint8_t tmp[512];
        if (HAL_SD_ReadBlocks(&hsd1, tmp, sector, 1, SD_READ_TIMEOUT) != HAL_OK)
        {
            return false;
        }
        while (HAL_SD_GetCardState(&hsd1) != HAL_SD_CARD_TRANSFER)
        {
        }
        memcpy((uint8_t *)buffer, tmp + offset, size);
        return true;
    }

    bool SdmmcDriverStm32::write(uint32_t sector, uint32_t offset, const void *buffer, size_t size)
    {
        if (!isReady() || buffer == nullptr)
        {
            return false;
        }

        if (size == 0)
        {
            return true;
        }

        const uint32_t ss = sectorSize();
        if (ss != kSectorSizeDefault || offset >= ss || (offset + size) > ss)
        {
            return false;
        }

        // Fast path: full sector write.
        if (offset == 0 && size == ss)
        {
            const bool aligned = ((((uintptr_t)buffer) & 0x3U) == 0);
            if (aligned)
            {
                if (HAL_SD_WriteBlocks(&hsd1, (uint8_t *)buffer, sector, 1, SD_WRITE_TIMEOUT) != HAL_OK)
                {
                    return false;
                }
                while (HAL_SD_GetCardState(&hsd1) != HAL_SD_CARD_TRANSFER)
                {
                }
                return true;
            }

            alignas(4) uint8_t tmp[512];
            memcpy(tmp, (const uint8_t *)buffer, ss);
            if (HAL_SD_WriteBlocks(&hsd1, tmp, sector, 1, SD_WRITE_TIMEOUT) != HAL_OK)
            {
                return false;
            }
            while (HAL_SD_GetCardState(&hsd1) != HAL_SD_CARD_TRANSFER)
            {
            }
            return true;
        }

        // Partial sector: read-modify-write.
        alignas(4) uint8_t tmp[512];
        if (HAL_SD_ReadBlocks(&hsd1, tmp, sector, 1, SD_READ_TIMEOUT) != HAL_OK)
        {
            return false;
        }
        while (HAL_SD_GetCardState(&hsd1) != HAL_SD_CARD_TRANSFER)
        {
        }

        memcpy(tmp + offset, (const uint8_t *)buffer, size);

        if (HAL_SD_WriteBlocks(&hsd1, tmp, sector, 1, SD_WRITE_TIMEOUT) != HAL_OK)
        {
            return false;
        }
        while (HAL_SD_GetCardState(&hsd1) != HAL_SD_CARD_TRANSFER)
        {
        }
        return true;
    }

} // namespace seeedfs

#else

namespace seeedfs
{

    SdmmcDriverStm32::SdmmcDriverStm32() : _ready(false), _sectorSize(0), _sectorCount(0), _type(SdCardType::None) {}
    SdmmcDriverStm32::~SdmmcDriverStm32() {}
    bool SdmmcDriverStm32::begin() { return false; }
    void SdmmcDriverStm32::end() {}
    bool SdmmcDriverStm32::isReady() const { return false; }
    uint32_t SdmmcDriverStm32::sectorSize() const { return 0; }
    uint32_t SdmmcDriverStm32::sectorCount() const { return 0; }
    SdCardType SdmmcDriverStm32::cardType() const { return SdCardType::None; }
    bool SdmmcDriverStm32::read(uint32_t, uint32_t, void *, size_t) { return false; }
    bool SdmmcDriverStm32::write(uint32_t, uint32_t, const void *, size_t) { return false; }

} // namespace seeedfs

#endif
