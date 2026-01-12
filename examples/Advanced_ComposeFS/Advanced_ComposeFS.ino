/*
  Advanced_ComposeFS

  Advanced usage: compose storage + filesystem manually.

  This example demonstrates the underlying "new architecture":
    StorageImpl (SdSpiDriver OR SdmmcStorage) + FSImpl (FatFs) + unified FS API (FS).

  When to use this pattern:
  - You are porting FatFs to a new storage backend.
  - You want direct access to the underlying driver (e.g., cardType, custom erase/sync).

  For most users, prefer:
    - SPI SD:    seeedfs::SdSpiFatFs
    - SDMMC:     seeedfs::SdmmcFatFs (WIO_LITE_AI)
*/

#include <Seeed_Arduino_FS.h>

#define LOG Serial

#ifndef SDCARD_SS_PIN
#define SDCARD_SS_PIN D2
#endif

#ifndef SDCARD_SPI
#define SDCARD_SPI SPI
#endif

#ifndef SDCARD_SPI_HZ
#define SDCARD_SPI_HZ 4000000UL
#endif

// Seeed XIAO (ESP32 family) default SPI pins
#ifndef SDCARD_SCK_PIN
#define SDCARD_SCK_PIN  D8
#endif
#ifndef SDCARD_MISO_PIN
#define SDCARD_MISO_PIN D9
#endif
#ifndef SDCARD_MOSI_PIN
#define SDCARD_MOSI_PIN D10
#endif

static void listDir(seeedfs::FS &filesystem, const char *dirname, int depth)
{
    seeedfs::Dir dir = filesystem.openDir(dirname);
    while (dir.next())
    {
        for (int i = 0; i < depth; i++)
        {
            LOG.print('\t');
        }

        const char *name = dir.fileName();
        LOG.print(name);
        if (dir.isDirectory())
        {
            LOG.println("/");
        }
        else
        {
            LOG.print("\t\t");
            LOG.print((uint32_t)dir.fileSize());
            LOG.println(" bytes");
        }
    }
}

static void printFSInfo(seeedfs::FS &filesystem)
{
    seeedfs::FSInfo info;
    if (!filesystem.info(info))
    {
        LOG.println("FS info failed");
        return;
    }

    LOG.print("Total space: ");
    LOG.print((uint32_t)(info.totalBytes / (1024 * 1024)));
    LOG.println("MB");

    LOG.print("Used space: ");
    LOG.print((uint32_t)(info.usedBytes / (1024 * 1024)));
    LOG.println("MB");
}

void setup()
{
    LOG.begin(115200);
    while (!LOG) {}

    LOG.println("\n===== Seeed_Arduino_FS Advanced_ComposeFS =====");

#ifdef WIO_LITE_AI
    // SDMMC path (WIO_LITE_AI)
    static seeedfs::SdmmcStorage storage;
    static seeedfs::FatFs fatfs(&storage);
    static seeedfs::FS fs(&fatfs);

    LOG.println("Backend: SDMMC (SdmmcStorage + FatFs + FS)");

    if (!storage.begin())
    {
        LOG.println("storage.begin() failed");
        return;
    }

    if (!fs.begin())
    {
        LOG.println("fs.begin() failed");
        storage.end();
        return;
    }

#else
    // SPI SD path (most boards)
    static seeedfs::SdSpiDriver storage;
    static seeedfs::FatFs fatfs(&storage);
    static seeedfs::FS fs(&fatfs);

    LOG.println("Backend: SPI SD (SdSpiDriver + FatFs + FS)");

#if defined(ARDUINO_ARCH_ESP32)
    SDCARD_SPI.begin(SDCARD_SCK_PIN, SDCARD_MISO_PIN, SDCARD_MOSI_PIN, SDCARD_SS_PIN);
#endif

    storage.configure(SDCARD_SS_PIN, SDCARD_SPI, SDCARD_SPI_HZ);

    if (!storage.begin())
    {
        LOG.println("storage.begin() failed");
        return;
    }

    if (!fs.begin())
    {
        LOG.println("fs.begin() failed");
        storage.end();
        return;
    }
#endif

    LOG.print("Storage type: ");
    LOG.println(storage.type());
    LOG.print("Capacity: ");
    LOG.print((uint32_t)(storage.size() / (1024 * 1024)));
    LOG.println("MB");

    listDir(fs, "/", 0);

    // Small write/read
    {
        seeedfs::File f = fs.open("/compose.txt", "w");
        if (!f)
        {
            LOG.println("open /compose.txt failed");
        }
        else
        {
            f.println("Hello from manual composition: StorageImpl + FatFs + FS");
            f.close();
            LOG.println("Write OK: /compose.txt");
        }

        seeedfs::File r = fs.open("/compose.txt", "r");
        if (r)
        {
            LOG.println("Read back:");
            while (r.available())
            {
                LOG.write(r.read());
            }
            r.close();
            LOG.println();
        }
        else
        {
            LOG.println("open /compose.txt for read failed");
        }
    }

    printFSInfo(fs);

    // Graceful shutdown (optional; many sketches just keep it mounted)
    fs.end();
    storage.end();

    LOG.println("Done.");
}

void loop() {}
