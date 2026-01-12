/*
  SD_QuickStart

  Goal: minimum-friction usage for Seeed_Arduino_FS.
        - Uses seeedfs::SdSpiFatFs (recommended)
  - Keeps SPI setup non-intrusive by default

  Notes:
  - On ESP32 family, if you need explicit SPI pins, use the beginSD(...) overload
    that includes SCK/MISO/MOSI.
*/

#include <Seeed_Arduino_FS.h>

#define LOG Serial

#ifndef SDCARD_SS_PIN
#define SDCARD_SS_PIN D2
#endif

// Seeed XIAO series default SPI pins for SD (ESP32 family)
#ifndef SDCARD_SCK_PIN
#define SDCARD_SCK_PIN  D8
#endif
#ifndef SDCARD_MISO_PIN
#define SDCARD_MISO_PIN D9
#endif
#ifndef SDCARD_MOSI_PIN
#define SDCARD_MOSI_PIN D10
#endif

#ifndef SDCARD_SPI
#define SDCARD_SPI SPI
#endif

// Conservative default for stability.
#ifndef SDCARD_SPI_HZ
#define SDCARD_SPI_HZ 400000UL
#endif

static void listRoot(seeedfs::FS &filesystem)
{
    seeedfs::Dir dir = filesystem.openDir("/");
    while (dir.next())
    {
        LOG.print(dir.fileName());
        if (dir.isDirectory())
        {
            LOG.println("/");
        }
        else
        {
            LOG.print("\t");
            LOG.print((uint32_t)dir.fileSize());
            LOG.println(" bytes");
        }
    }
}

void setup()
{
    LOG.begin(115200);
    while (!LOG) {}

    LOG.println("\n===== Seeed_Arduino_FS SD_QuickStart =====");

    static seeedfs::SdSpiFatFs sd;

#if defined(ARDUINO_ARCH_ESP32)
    LOG.print("Using SPI pins SCK/MISO/MOSI = ");
    LOG.print(SDCARD_SCK_PIN);
    LOG.print('/');
    LOG.print(SDCARD_MISO_PIN);
    LOG.print('/');
    LOG.println(SDCARD_MOSI_PIN);

    seeedfs::SdSpiCfg cfg;
    cfg.csPin = SDCARD_SS_PIN;
    cfg.spi = &SDCARD_SPI;
    cfg.spiHz = SDCARD_SPI_HZ;
    cfg.sckPin = SDCARD_SCK_PIN;
    cfg.misoPin = SDCARD_MISO_PIN;
    cfg.mosiPin = SDCARD_MOSI_PIN;

    if (!sd.begin(cfg))
#else
    seeedfs::SdSpiCfg cfg;
    cfg.csPin = SDCARD_SS_PIN;
    cfg.spi = &SDCARD_SPI;
    cfg.spiHz = SDCARD_SPI_HZ;

    if (!sd.begin(cfg))
#endif
    {
        LOG.print("SD init failed: ");
        LOG.println(sd.lastErrorString());
        return;
    }

    seeedfs::FS &filesystem = sd.fs();

    listRoot(filesystem);

    seeedfs::File f = filesystem.open("/hello.txt", "a");
    if (!f)
    {
        LOG.println("open /hello.txt failed");
        return;
    }
    f.println("Hello from seeedfs::SdSpiFatFs");
    f.close();

    LOG.println("Done.");
}

void loop()
{
}
