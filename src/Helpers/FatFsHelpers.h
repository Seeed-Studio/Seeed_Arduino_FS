#pragma once

#include <Arduino.h>

#include "Seeed_FS.h"
#include "FileSystem/FATFS/FatFs.h"
#include "Drivers/SdCard/SdSpiDriver.h"
#include "Storage/SdCard/SdmmcStorage.h"

namespace seeedfs
{

    struct SdSpiCfg
    {
        uint8_t csPin = 0;
        SPIClass *spi = &SPI;
        uint32_t spiHz = 400000UL;

        // Optional explicit pins (ESP32 family). If any pin is < 0, pins are ignored.
        int sckPin = -1;
        int misoPin = -1;
        int mosiPin = -1;

        // If true, will call spi->begin(...) when explicit pins are provided.
        // Default: true (only when pins are explicitly set), otherwise no begin.
        bool autoSpiBeginWhenPinsProvided = true;
    };

    struct SdmmcCfg
    {
    };

    class SdSpiFatFs
    {
    public:
        enum class Error : uint8_t
        {
            Ok = 0,
            InvalidConfig,
            StorageInitFailed,
            FsBeginFailed,
        };

    public:
        SdSpiFatFs()
            : _sdSpi(),
              _fatfs(&_sdSpi),
              _fs(&_fatfs),
              _lastError(Error::Ok)
        {
        }

        bool begin(const SdSpiCfg &cfg)
        {
            setLastError(Error::Ok);

            if (cfg.spi == nullptr)
            {
                setLastError(Error::InvalidConfig);
                return false;
            }

#if defined(ARDUINO_ARCH_ESP32)
            const bool pinsProvided = (cfg.sckPin >= 0) && (cfg.misoPin >= 0) && (cfg.mosiPin >= 0);
            if (cfg.autoSpiBeginWhenPinsProvided && pinsProvided)
            {
                cfg.spi->begin(cfg.sckPin, cfg.misoPin, cfg.mosiPin, cfg.csPin);
            }
#endif

            _sdSpi.configure(cfg.csPin, *cfg.spi, cfg.spiHz);
            if (!_sdSpi.begin())
            {
                setLastError(Error::StorageInitFailed);
                return false;
            }

            if (!_fs.begin())
            {
                setLastError(Error::FsBeginFailed);
                return false;
            }
            setLastError(Error::Ok);
            return true;
        }

        void end()
        {
            _fs.end();
            _sdSpi.end();
        }
        bool format() { return _fs.format(); }
        bool info(FSInfo &info) { return _fs.info(info); }
        bool setConfig(const FSConfig &cfg) { return _fs.setConfig(cfg); }

        File open(const char *path, const char *mode = "r") { return _fs.open(path, mode); }
        bool exists(const char *path) { return _fs.exists(path); }
        Dir openDir(const char *path) { return _fs.openDir(path); }
        bool remove(const char *path) { return _fs.remove(path); }
        bool rename(const char *pathFrom, const char *pathTo) { return _fs.rename(pathFrom, pathTo); }
        bool mkdir(const char *path) { return _fs.mkdir(path); }
        bool rmdir(const char *path) { return _fs.rmdir(path); }

        const char *lastErrorString() const
        {
            switch (_lastError)
            {
            case Error::Ok:
                return "Ok";
            case Error::InvalidConfig:
                return "Invalid Config";
            case Error::StorageInitFailed:
                return "Storage Init Failed";
            case Error::FsBeginFailed:
                return "FS Begin Failed";
            default:
                return "Unknown Error";
            }
        }

        uint64_t size() const { return _sdSpi.size(); }
        const char *type() const { return _sdSpi.type(); }

        FS &fs() { return _fs; }
        FatFs &fatfs() { return _fatfs; }
        SdSpiDriver &driver() { return _sdSpi; }

        Error lastError() const { return _lastError; }

    private:
        void setLastError(Error err) { _lastError = err; }

        SdSpiDriver _sdSpi;
        FatFs _fatfs;
        FS _fs;
        Error _lastError;
    };

#ifdef WIO_LITE_AI

    class SdmmcFatFs
    {
    public:
        enum class Error : uint8_t
        {
            Ok = 0,
            StorageInitFailed,
            FsBeginFailed,
        };

    public:
        SdmmcFatFs()
            : _sdmmc(),
              _fatfs(&_sdmmc),
              _fs(&_fatfs),
              _lastError(Error::Ok)
        {
        }

        bool begin(const SdmmcCfg &cfg = SdmmcCfg())
        {
            (void)cfg;
            setLastError(Error::Ok);

            if (!_sdmmc.begin())
            {
                setLastError(Error::StorageInitFailed);
                return false;
            }

            if (!_fs.begin())
            {
                setLastError(Error::FsBeginFailed);
                return false;
            }

            setLastError(Error::Ok);
            return true;
        }

        void end()
        {
            _fs.end();
            _sdmmc.end();
        }

        bool format() { return _fs.format(); }
        bool info(FSInfo &info) { return _fs.info(info); }
        bool setConfig(const FSConfig &cfg) { return _fs.setConfig(cfg); }

        File open(const char *path, const char *mode = "r") { return _fs.open(path, mode); }
        bool exists(const char *path) { return _fs.exists(path); }
        Dir openDir(const char *path) { return _fs.openDir(path); }
        bool remove(const char *path) { return _fs.remove(path); }
        bool rename(const char *pathFrom, const char *pathTo) { return _fs.rename(pathFrom, pathTo); }
        bool mkdir(const char *path) { return _fs.mkdir(path); }
        bool rmdir(const char *path) { return _fs.rmdir(path); }

        const char *lastErrorString() const
        {
            switch (_lastError)
            {
            case Error::Ok:
                return "Ok";
            case Error::StorageInitFailed:
                return "Storage Init Failed";
            case Error::FsBeginFailed:
                return "FS Begin Failed";
            default:
                return "Unknown Error";
            }
        }

        uint64_t size() const { return _sdmmc.size(); }
        const char *type() const { return _sdmmc.type(); }

        FS &fs() { return _fs; }
        FatFs &fatfs() { return _fatfs; }
        SdmmcStorage &driver() { return _sdmmc; }

        Error lastError() const { return _lastError; }

    private:
        void setLastError(Error err) { _lastError = err; }

        SdmmcStorage _sdmmc;
        FatFs _fatfs;
        FS _fs;
        Error _lastError;
    };

#endif

} // namespace seeedfs
