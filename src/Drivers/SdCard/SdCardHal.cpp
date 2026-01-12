#include "SdCardHal.h"

#include "Utils/Errors.h"

// SD card command enumeration
typedef enum
{
    GO_IDLE_STATE = 0,           // Enter idle state
    SEND_OP_COND = 1,            // Send operation condition
    SEND_CID = 2,                // Send card identification
    SEND_RELATIVE_ADDR = 3,      // Send relative address
    SEND_SWITCH_FUNC = 6,        // Send switch function
    SEND_IF_COND = 8,            // Send interface condition
    SEND_CSD = 9,                // Send card specific data
    STOP_TRANSMISSION = 12,      // Stop transmission
    SEND_STATUS = 13,            // Send status
    SET_BLOCKLEN = 16,           // Set block length
    READ_BLOCK_SINGLE = 17,      // Read single block
    READ_BLOCK_MULTIPLE = 18,    // Read multiple blocks
    SEND_NUM_WR_BLOCKS = 22,     // Send number of write blocks
    SET_WR_BLK_ERASE_COUNT = 23, // Set write block erase count
    WRITE_BLOCK_SINGLE = 24,     // Write single block
    WRITE_BLOCK_MULTIPLE = 25,   // Write multiple blocks
    APP_OP_COND = 41,            // Application specific operation condition
    APP_CLR_CARD_DETECT = 42,    // Clear card detect
    APP_CMD = 55,                // Application command prefix
    READ_OCR = 58,               // Read operation condition register
    CRC_ON_OFF = 59              // CRC on/off
} sdcard_command_t;

namespace seeedfs
{

// On ESP32 platform, users often specify pins in sketch with SPI.begin(SCK,MISO,MOSI,CS).
// If HAL internally calls SPI.begin() (without parameters) again, it will reset pins to default values, causing communication anomalies.
#ifndef SEEED_SDCARD_AUTO_SPI_BEGIN
#if defined(ARDUINO_ARCH_ESP32)
#define SEEED_SDCARD_AUTO_SPI_BEGIN 0
#else
#define SEEED_SDCARD_AUTO_SPI_BEGIN 1
#endif
#endif

    static const uint8_t CRC7Table[256] = {
        0x00, 0x09, 0x12, 0x1B, 0x24, 0x2D, 0x36, 0x3F,
        0x48, 0x41, 0x5A, 0x53, 0x6C, 0x65, 0x7E, 0x77,
        0x19, 0x10, 0x0B, 0x02, 0x3D, 0x34, 0x2F, 0x26,
        0x51, 0x58, 0x43, 0x4A, 0x75, 0x7C, 0x67, 0x6E,
        0x32, 0x3B, 0x20, 0x29, 0x16, 0x1F, 0x04, 0x0D,
        0x7A, 0x73, 0x68, 0x61, 0x5E, 0x57, 0x4C, 0x45,
        0x2B, 0x22, 0x39, 0x30, 0x0F, 0x06, 0x1D, 0x14,
        0x63, 0x6A, 0x71, 0x78, 0x47, 0x4E, 0x55, 0x5C,
        0x64, 0x6D, 0x76, 0x7F, 0x40, 0x49, 0x52, 0x5B,
        0x2C, 0x25, 0x3E, 0x37, 0x08, 0x01, 0x1A, 0x13,
        0x7D, 0x74, 0x6F, 0x66, 0x59, 0x50, 0x4B, 0x42,
        0x35, 0x3C, 0x27, 0x2E, 0x11, 0x18, 0x03, 0x0A,
        0x56, 0x5F, 0x44, 0x4D, 0x72, 0x7B, 0x60, 0x69,
        0x1E, 0x17, 0x0C, 0x05, 0x3A, 0x33, 0x28, 0x21,
        0x4F, 0x46, 0x5D, 0x54, 0x6B, 0x62, 0x79, 0x70,
        0x07, 0x0E, 0x15, 0x1C, 0x23, 0x2A, 0x31, 0x38,
        0x41, 0x48, 0x53, 0x5A, 0x65, 0x6C, 0x77, 0x7E,
        0x09, 0x00, 0x1B, 0x12, 0x2D, 0x24, 0x3F, 0x36,
        0x58, 0x51, 0x4A, 0x43, 0x7C, 0x75, 0x6E, 0x67,
        0x10, 0x19, 0x02, 0x0B, 0x34, 0x3D, 0x26, 0x2F,
        0x73, 0x7A, 0x61, 0x68, 0x57, 0x5E, 0x45, 0x4C,
        0x3B, 0x32, 0x29, 0x20, 0x1F, 0x16, 0x0D, 0x04,
        0x6A, 0x63, 0x78, 0x71, 0x4E, 0x47, 0x5C, 0x55,
        0x22, 0x2B, 0x30, 0x39, 0x06, 0x0F, 0x14, 0x1D,
        0x25, 0x2C, 0x37, 0x3E, 0x01, 0x08, 0x13, 0x1A,
        0x6D, 0x64, 0x7F, 0x76, 0x49, 0x40, 0x5B, 0x52,
        0x3C, 0x35, 0x2E, 0x27, 0x18, 0x11, 0x0A, 0x03,
        0x74, 0x7D, 0x66, 0x6F, 0x50, 0x59, 0x42, 0x4B,
        0x17, 0x1E, 0x05, 0x0C, 0x33, 0x3A, 0x21, 0x28,
        0x5F, 0x56, 0x4D, 0x44, 0x7B, 0x72, 0x69, 0x60,
        0x0E, 0x07, 0x1C, 0x15, 0x2A, 0x23, 0x38, 0x31,
        0x46, 0x4F, 0x54, 0x5D, 0x62, 0x6B, 0x70, 0x79};

    static const uint16_t CRC16Table[256] = {
        0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
        0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
        0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
        0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
        0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
        0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
        0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
        0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
        0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
        0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
        0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
        0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
        0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
        0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
        0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
        0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
        0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
        0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
        0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
        0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
        0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
        0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
        0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
        0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
        0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
        0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
        0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
        0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
        0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
        0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
        0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
        0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0};

    SdCardHal::SdCardHal(uint8_t cs, SPIClass &spi, int hz)
        : _cs(cs), _spi(spi), _hz(hz), _type(SdCardType::None), _sectors(0), _supports_crc(false), _initialized(false) {}

    SdCardHal::~SdCardHal()
    {
        uninit();
    }

    bool SdCardHal::init()
    {
        if (_initialized)
            return true;

        // Auto-initialize SPI only when needed (ESP32 defaults to not auto begin to avoid overriding user-defined pins)
#if SEEED_SDCARD_AUTO_SPI_BEGIN
        _spi.begin();
#endif

        pinMode(_cs, OUTPUT);
        digitalWrite(_cs, HIGH);

        // Initialization phase uniformly uses 400kHz SPI transaction
        SpiTransaction lock(&_spi, 400000);

        // Send idle bytes (at least 74 clocks)
        for (uint8_t i = 0; i < 20; i++)
        {
            _spi.transfer(0xFF);
        }

        uint8_t token;
        unsigned int resp;

        // Send CMD0 to put SD card into idle state
        token = sdTransaction(GO_IDLE_STATE, 0, nullptr);
        if (token != 1)
        {
            seeedfs_reportErrorDetail(SEEED_FS_ERR_SD_CMD0_FAILED, (int32_t)token);
            return false;
        }

        // Default not to enable CRC (in SPI mode, CRC is not required except for CMD0/CMD8; enabling it may cause CRC error if CRC7 algorithm doesn't match)
        _supports_crc = false;

        // ACMD41/CMD1 common parameter: voltage window 2.7-3.6V
        static const uint32_t OCR_VOLTAGE_WINDOW = 0x00FF8000UL;

        // Check if SDHC/SDXC card (CMD8)
        token = sdTransaction(SEND_IF_COND, 0x1AA, &resp);
        if (token == 1)
        {
            if ((resp & 0xFFF) != 0x1AA)
            {
                seeedfs_reportErrorDetail(SEEED_FS_ERR_SD_CMD8_FAILED, (int32_t)resp);
                return false;
            }

            // Send ACMD41 to activate SD card
            uint32_t start = millis();
            do
            {
                token = sdTransaction(APP_OP_COND, 0x40000000UL | OCR_VOLTAGE_WINDOW, nullptr);
            } while (token == 1 && (millis() - start) < 2000);

            if (token != 0)
            {
                seeedfs_reportErrorDetail(SEEED_FS_ERR_SD_ACMD41_FAILED, (int32_t)token);
                return false;
            }

            // Check if SDHC
            token = sdTransaction(READ_OCR, 0, &resp);
            if (token != 0 || resp == 0xFFFFFFFF)
            {
                const int32_t detail = (resp == 0xFFFFFFFF) ? (int32_t)token : (int32_t)resp;
                seeedfs_reportErrorDetail(SEEED_FS_ERR_SD_READ_OCR_FAILED, detail);
                return false;
            }
            _type = (resp & (1UL << 30)) ? SdCardType::Sdhc : SdCardType::Sd;
        }
        else
        {
            // Handle SD v1 or MMC card
            // Note: On some cards/compatibility paths, CMD58(READ_OCR) may return ILLEGAL_COMMAND,
            // should not force failure before ACMD41/CMD1.
            uint32_t start = millis();
            do
            {
                token = sdTransaction(APP_OP_COND, OCR_VOLTAGE_WINDOW, nullptr);
            } while (token == 0x01 && (millis() - start) < 2000);

            if (token == 0)
            {
                _type = SdCardType::Sd;
            }
            else
            {
                start = millis();
                do
                {
                    token = sdTransaction(SEND_OP_COND, OCR_VOLTAGE_WINDOW, nullptr);
                } while (token != 0x00 && (millis() - start) < 2000);

                if (token == 0x00)
                {
                    _type = SdCardType::Mmc;
                }
                else
                {
                    seeedfs_reportErrorDetail(SEEED_FS_ERR_SD_CMD1_FAILED, (int32_t)token);
                    return false;
                }
            }
        }

        // Disable card detect (SD cards only)
        if (_type != SdCardType::Mmc)
        {
            const uint8_t t = sdTransaction(APP_CLR_CARD_DETECT, 0, nullptr);
            if (t)
            {
                seeedfs_reportErrorDetail(SEEED_FS_ERR_SD_ILLEGAL_COMMAND, (int32_t)t);
                return false;
            }
        }

        // Set block length to 512 bytes (non-SDHC cards)
        if (_type != SdCardType::Sdhc)
        {
            const uint8_t t = sdTransaction(SET_BLOCKLEN, 512, nullptr);
            if (t != 0x00)
            {
                seeedfs_reportErrorDetail(SEEED_FS_ERR_SD_SET_BLOCKLEN_FAILED, (int32_t)t);
                return false;
            }
        }

        // Get total sector count
        _sectors = sdGetSectorsCount();
        if (_sectors == 0)
        {
            if (seeedfs_lastError() == SEEED_FS_OK)
            {
                seeedfs_reportError(SEEED_FS_ERR_SD_GET_SECTOR_COUNT_FAILED);
            }
            return false;
        }

        // Limit max frequency to 25 MHz
        if (_hz > 25000000)
            _hz = 25000000;

        _initialized = true;
        return true;
    }

    void SdCardHal::uninit()
    {
        _initialized = false;
        _type = SdCardType::None;
        _sectors = 0;
        digitalWrite(_cs, HIGH);
    }

    bool SdCardHal::readSector(uint32_t sector, uint8_t *buffer)
    {
        if (!_initialized)
        {
            seeedfs_reportError(SEEED_FS_ERR_SD_NOT_INITIALIZED);
            return false;
        }

        SpiTransaction lock(&_spi, _hz);
        if (!sdSelectCard())
        {
            seeedfs_reportError(SEEED_FS_ERR_SD_CARD_SELECTION_TIMEOUT);
            return false;
        }

        unsigned int addr = (_type == SdCardType::Sdhc) ? sector : (sector << 9);
        if (sdCommand(READ_BLOCK_SINGLE, addr, nullptr) == 0)
        {
            bool success = sdReadBytes((char *)buffer, 512);
            sdDeselectCard();
            if (!success)
            {
                seeedfs_reportErrorDetail(SEEED_FS_ERR_SD_READ_FAILED, (int32_t)sector);
            }
            return success;
        }
        seeedfs_reportErrorDetail(SEEED_FS_ERR_SD_CMD17_FAILED, (int32_t)sector);
        sdDeselectCard();
        return false;
    }

    bool SdCardHal::readSectors(uint32_t sector, uint8_t *buffer, uint32_t count)
    {
        if (!_initialized)
        {
            seeedfs_reportError(SEEED_FS_ERR_SD_NOT_INITIALIZED);
            return false;
        }
        if (count == 0)
        {
            seeedfs_reportError(SEEED_FS_ERR_SD_INVALID_COUNT);
            return false;
        }

        SpiTransaction lock(&_spi, _hz);
        if (!sdSelectCard())
        {
            seeedfs_reportError(SEEED_FS_ERR_SD_CARD_SELECTION_TIMEOUT);
            return false;
        }

        unsigned int addr = (_type == SdCardType::Sdhc) ? sector : (sector << 9);
        if (sdCommand(READ_BLOCK_MULTIPLE, addr, nullptr) == 0)
        {
            char *currentBuffer = (char *)buffer;
            do
            {
                if (!sdReadBytes(currentBuffer, 512))
                {
                    seeedfs_reportErrorDetail(SEEED_FS_ERR_SD_READ_FAILED, (int32_t)sector);
                    sdDeselectCard();
                    return false;
                }
                currentBuffer += 512;
            } while (--count);

            if (sdCommand(STOP_TRANSMISSION, 0, nullptr) != 0)
            {
                seeedfs_reportError(SEEED_FS_ERR_SD_CMD12_FAILED);
                sdDeselectCard();
                return false;
            }
            sdDeselectCard();
            return true;
        }
        seeedfs_reportErrorDetail(SEEED_FS_ERR_SD_CMD18_FAILED, (int32_t)sector);
        sdDeselectCard();
        return false;
    }

    bool SdCardHal::writeSector(uint32_t sector, const uint8_t *buffer)
    {
        if (!_initialized)
        {
            seeedfs_reportError(SEEED_FS_ERR_SD_NOT_INITIALIZED);
            return false;
        }

        SpiTransaction lock(&_spi, _hz);
        if (!sdSelectCard())
        {
            seeedfs_reportError(SEEED_FS_ERR_SD_CARD_SELECTION_TIMEOUT);
            return false;
        }

        unsigned int addr = (_type == SdCardType::Sdhc) ? sector : (sector << 9);
        if (sdCommand(WRITE_BLOCK_SINGLE, addr, nullptr) == 0)
        {
            uint8_t token = sdWriteBytes((const char *)buffer, 0xFE);
            sdDeselectCard();
            if (token != 0x05)
            {
                seeedfs_reportErrorDetail(SEEED_FS_ERR_SD_INVALID_TOKEN, (int32_t)token);
                return false;
            }

            unsigned int resp;
            if (sdTransaction(SEND_STATUS, 0, &resp) || resp)
            {
                seeedfs_reportErrorDetail(SEEED_FS_ERR_SD_SEND_STATUS_FAILED, (int32_t)resp);
                return false;
            }
            return true;
        }
        seeedfs_reportErrorDetail(SEEED_FS_ERR_SD_CMD24_FAILED, (int32_t)sector);
        sdDeselectCard();
        return false;
    }

    bool SdCardHal::writeSectors(uint32_t sector, const uint8_t *buffer, uint32_t count)
    {
        if (!_initialized)
        {
            seeedfs_reportError(SEEED_FS_ERR_SD_NOT_INITIALIZED);
            return false;
        }
        if (count == 0)
        {
            seeedfs_reportError(SEEED_FS_ERR_SD_INVALID_COUNT);
            return false;
        }

        SpiTransaction lock(&_spi, _hz);

        // Set write block count (non-MMC cards)
        if (_type != SdCardType::Mmc)
        {
            if (sdTransaction(SET_WR_BLK_ERASE_COUNT, count, nullptr))
            {
                seeedfs_reportError(SEEED_FS_ERR_SD_SET_WR_BLK_ERASE_COUNT_FAILED);
                return false;
            }
        }

        if (!sdSelectCard())
        {
            seeedfs_reportError(SEEED_FS_ERR_SD_CARD_SELECTION_TIMEOUT);
            return false;
        }

        unsigned int addr = (_type == SdCardType::Sdhc) ? sector : (sector << 9);
        if (sdCommand(WRITE_BLOCK_MULTIPLE, addr, nullptr) == 0)
        {
            const char *currentBuffer = (const char *)buffer;
            do
            {
                uint8_t token = sdWriteBytes(currentBuffer, 0xFC);
                if (token != 0x05)
                {
                    seeedfs_reportErrorDetail(SEEED_FS_ERR_SD_INVALID_TOKEN, (int32_t)token);
                    sdStop();
                    sdDeselectCard();
                    return false;
                }
                currentBuffer += 512;
            } while (--count);

            if (!sdWait(500))
            {
                seeedfs_reportError(SEEED_FS_ERR_SD_TIMEOUT_WAITING_WRITE_COMPLETION);
                sdDeselectCard();
                return false;
            }

            sdStop();
            sdDeselectCard();

            unsigned int resp;
            if (sdTransaction(SEND_STATUS, 0, &resp) || resp)
            {
                seeedfs_reportErrorDetail(SEEED_FS_ERR_SD_SEND_STATUS_FAILED, (int32_t)resp);
                return false;
            }
            return true;
        }
        seeedfs_reportErrorDetail(SEEED_FS_ERR_SD_CMD25_FAILED, (int32_t)sector);
        sdDeselectCard();
        return false;
    }

    uint32_t SdCardHal::getSectors() const
    {
        return _initialized ? _sectors : 0;
    }

    SdCardType SdCardHal::getType() const
    {
        return _type;
    }

    // CRC calculation functions
    uint8_t SdCardHal::crc7(const uint8_t *data, size_t length)
    {
        // Standard SD CRC7 (polynomial x^7 + x^3 + 1).
        // Return value is "on-wire format": (crc7 << 1) | 1.
        uint8_t crc = 0;
        for (size_t i = 0; i < length; i++)
        {
            uint8_t d = data[i];
            for (uint8_t bit = 0; bit < 8; bit++)
            {
                crc <<= 1;
                if (((d & 0x80) ^ (crc & 0x80)) != 0)
                {
                    crc ^= 0x09;
                }
                d <<= 1;
            }
            crc &= 0x7F;
        }
        return (crc << 1) | 0x01;
    }

    uint16_t SdCardHal::crc16(const uint8_t *data, size_t length)
    {
        uint16_t crc = 0;
        for (size_t i = 0; i < length; i++)
        {
            crc = (crc << 8) ^ CRC16Table[((crc >> 8) ^ data[i]) & 0xFF];
        }
        return crc;
    }

    // Internal helper functions
    bool SdCardHal::sdWait(int timeout)
    {
        uint8_t resp;
        uint32_t start = millis();
        do
        {
            resp = _spi.transfer(0xFF);
            // SD SPI returns 0xFF when ready; returns 0x00 continuously when busy
            if (resp == 0xFF)
            {
                return true;
            }
        } while ((millis() - start) < (unsigned int)timeout);

        seeedfs_reportErrorDetail(SEEED_FS_ERR_SD_WAIT_TIMEOUT, (int32_t)timeout);
        return false;
    }

    bool SdCardHal::sdSelectCard()
    {
        digitalWrite(_cs, LOW);
        // Send an idle clock after selecting card to avoid command boundary misalignment
        _spi.transfer(0xFF);
        return sdWait(300);
    }

    void SdCardHal::sdDeselectCard()
    {
        digitalWrite(_cs, HIGH);
        _spi.transfer(0xFF); // Send an idle byte
    }

    void SdCardHal::sdStop()
    {
        _spi.transfer(0xFD); // Send stop token
    }

    uint8_t SdCardHal::sdCommand(uint8_t cmd, unsigned int arg, unsigned int *resp)
    {
        uint8_t cmdPacket[7];
        cmdPacket[0] = cmd | 0x40; // Add 0x40 before command
        cmdPacket[1] = arg >> 24;
        cmdPacket[2] = arg >> 16;
        cmdPacket[3] = arg >> 8;
        cmdPacket[4] = arg;

        if (cmd == GO_IDLE_STATE)
        {
            cmdPacket[5] = 0x95; // Fixed CRC for CMD0
        }
        else if (cmd == SEND_IF_COND)
        {
            cmdPacket[5] = 0x87; // Fixed CRC for CMD8
        }
        else
        {
            // Even if CRC is not explicitly enabled, sending correct CRC7 has no side effects;
            // but if the card validates CRC, it can avoid R1 returning CRC error.
            cmdPacket[5] = crc7(reinterpret_cast<const uint8_t *>(cmdPacket), 5);
        }

        // ESP32 SD driver practice: send an extra dummy byte only for CMD12
        cmdPacket[6] = 0xFF;
        _spi.transfer(cmdPacket, (cmd == STOP_TRANSMISSION) ? 7 : 6);

        uint8_t token = 0xFF;
        for (int i = 0; i < 9; i++)
        {
            token = _spi.transfer(0xFF);
            if (!(token & 0x80))
                break;
        }

        if (resp && (cmd == SEND_IF_COND || cmd == READ_OCR || cmd == SEND_STATUS))
        {
            *resp = 0;
            for (uint8_t i = 0; i < (cmd == SEND_STATUS ? 1 : 4); i++)
            {
                *resp = (*resp << 8) | _spi.transfer(0xFF);
            }
        }
        return token;
    }

    bool SdCardHal::sdReadBytes(char *buffer, int length)
    {
        uint8_t token;
        uint32_t start = millis();
        do
        {
            token = _spi.transfer(0xFF);
            if (token != 0xFE && (millis() - start) >= 500)
            {
                seeedfs_reportError(SEEED_FS_ERR_SD_READ_BYTES_TIMEOUT);
                return false;
            }
        } while (token == 0xFF);

        if (token != 0xFE)
        {
            seeedfs_reportErrorDetail(SEEED_FS_ERR_SD_INVALID_DATA_TOKEN, (int32_t)token);
            return false;
        }

        // Continuously send 0xFF when reading data to provide clock (more compatible with various platform SPI implementations)
        uint8_t *out = reinterpret_cast<uint8_t *>(buffer);
        for (int i = 0; i < length; i++)
        {
            out[i] = _spi.transfer(0xFF);
        }

        uint16_t crc = _spi.transfer(0xFF) << 8;
        crc |= _spi.transfer(0xFF);
        if (_supports_crc && crc != crc16(reinterpret_cast<const uint8_t *>(buffer), length))
        {
            seeedfs_reportErrorDetail(SEEED_FS_ERR_SD_CRC_ERROR, (int32_t)crc);
            return false;
        }
        return true;
    }

    uint8_t SdCardHal::sdWriteBytes(const char *buffer, uint8_t token)
    {
        if (!sdWait(500))
        {
            seeedfs_reportError(SEEED_FS_ERR_SD_WRITE_BYTES_TIMEOUT);
            return 0xFF;
        }

        _spi.transfer(token);
        for (int i = 0; i < 512; i++)
        {
            _spi.transfer(buffer[i]);
        }

        uint16_t crc = _supports_crc ? crc16((uint8_t *)buffer, 512) : 0xFFFF;
        _spi.transfer(crc >> 8);
        _spi.transfer(crc & 0xFF);

        return _spi.transfer(0xFF) & 0x1F; // Return data response
    }

    uint8_t SdCardHal::sdTransaction(uint8_t cmd, unsigned int arg, unsigned int *resp)
    {
        // Reference ESP32 SD library: retry a few times for no-token/CRC error, and toggle CS between CMD55 and ACMD.
        for (uint8_t attempt = 0; attempt < 3; attempt++)
        {
            if (!sdSelectCard())
            {
                seeedfs_reportErrorDetail(SEEED_FS_ERR_SD_TRANSACTION_TIMEOUT, (int32_t)cmd);
                sdDeselectCard();
                return 0xFF;
            }

            const bool isAcmd = (cmd == APP_OP_COND || cmd == APP_CLR_CARD_DETECT || cmd == SET_WR_BLK_ERASE_COUNT);
            if (isAcmd)
            {
                uint8_t pre = sdCommand(APP_CMD, 0, nullptr);
                sdDeselectCard();
                if (pre > 1)
                {
                    if (pre == 0xFF || (pre & 0x08))
                    {
                        delay(5);
                        continue;
                    }
                    seeedfs_reportErrorDetail(SEEED_FS_ERR_SD_APP_CMD_FAILED, (int32_t)pre);
                    return pre;
                }
                if (!sdSelectCard())
                {
                    sdDeselectCard();
                    delay(5);
                    continue;
                }
            }

            uint8_t token = sdCommand(cmd, arg, resp);
            sdDeselectCard();

            if (token == 0xFF || (token & 0x08))
            {
                delay(5);
                continue;
            }
            return token;
        }

        return 0xFF;
    }

    unsigned long SdCardHal::sdGetSectorsCount()
    {
        if (!sdSelectCard())
        {
            seeedfs_reportError(SEEED_FS_ERR_SD_CARD_SELECTION_TIMEOUT);
            return 0;
        }

        if (sdCommand(SEND_CSD, 0, nullptr) == 0)
        {
            char csd[16];
            if (sdReadBytes(csd, 16))
            {
                sdDeselectCard();
                if ((csd[0] >> 6) == 0x01)
                { // CSD v2 (SDHC/SDXC)
                    unsigned long size = (((unsigned long)(csd[7] & 0x3F) << 16) | ((unsigned long)csd[8] << 8) | csd[9]) + 1;
                    return size << 10;
                }
                else
                { // CSD v1 (SD/MMC)
                    unsigned long size = (((unsigned long)(csd[6] & 0x03) << 10) | ((unsigned long)csd[7] << 2) | ((csd[8] & 0xC0) >> 6)) + 1;
                    size <<= (((csd[9] & 0x03) << 1) | ((csd[10] & 0x80) >> 7)) + 2;
                    size <<= (csd[5] & 0x0F);
                    return size >> 9;
                }
            }
            seeedfs_reportError(SEEED_FS_ERR_SD_CSD_READ_FAILED);
        }
        else
        {
            seeedfs_reportError(SEEED_FS_ERR_SD_SEND_CSD_FAILED);
        }
        sdDeselectCard();
        return 0;
    }

} // namespace seeedfs