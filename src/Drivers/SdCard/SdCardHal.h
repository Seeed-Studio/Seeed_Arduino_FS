#ifndef SD_CARD_HAL_H
#define SD_CARD_HAL_H

#include "Drivers/SdCard/SdCardTypes.h"
#include <SPI.h>
#include <Arduino.h>

namespace seeedfs
{

    class SdCardHal
    {
    public:
        // Constructor: pass CS pin, SPI instance, and frequency
        SdCardHal(uint8_t cs, SPIClass &spi, int hz);
        ~SdCardHal();

        // Initialize SD card
        bool init();
        // Close SD card
        void uninit();
        // Read single 512-byte sector
        bool readSector(uint32_t sector, uint8_t *buffer);
        // Read multiple 512-byte sectors
        bool readSectors(uint32_t sector, uint8_t *buffer, uint32_t count);
        // Write single 512-byte sector
        bool writeSector(uint32_t sector, const uint8_t *buffer);
        // Write multiple 512-byte sectors
        bool writeSectors(uint32_t sector, const uint8_t *buffer, uint32_t count);
        // Get total sector count
        uint32_t getSectors() const;
        // Get SD card type
        SdCardType getType() const;

    private:
        uint8_t _cs;        // CS pin
        SPIClass &_spi;     // SPI instance reference
        int _hz;            // SPI clock frequency
        SdCardType _type;   // SD card type
        uint32_t _sectors;  // Sector count
        bool _supports_crc; // Supports CRC
        bool _initialized;  // Initialization status

        // CRC calculation function
        uint16_t crc16(const uint8_t *data, size_t length);
        uint8_t crc7(const uint8_t *data, size_t length);

        // Internal helper functions
        bool sdWait(int timeout);                                                 // Wait for SD card response
        bool sdSelectCard();                                                      // Select SD card
        void sdDeselectCard();                                                    // Deselect SD card
        void sdStop();                                                            // Stop transmission
        uint8_t sdCommand(uint8_t cmd, unsigned int arg, unsigned int *resp);     // Send SD card command
        bool sdReadBytes(char *buffer, int length);                               // Read data bytes
        uint8_t sdWriteBytes(const char *buffer, uint8_t token);                  // Write data bytes
        uint8_t sdTransaction(uint8_t cmd, unsigned int arg, unsigned int *resp); // Execute SD card transaction
        unsigned long sdGetSectorsCount();                                        // Get total sector count

        // SPI transaction management class
        struct SpiTransaction
        {
            SPIClass *spi;
            explicit SpiTransaction(SPIClass *spi, int frequency) : spi(spi)
            {
                spi->beginTransaction(SPISettings(frequency, MSBFIRST, SPI_MODE0));
            }
            ~SpiTransaction()
            {
                spi->endTransaction();
            }

        private:
            SpiTransaction(const SpiTransaction &) = delete;
            SpiTransaction &operator=(const SpiTransaction &) = delete;
        };
    };

} // namespace seeedfs

#endif // SD_CARD_HAL_H