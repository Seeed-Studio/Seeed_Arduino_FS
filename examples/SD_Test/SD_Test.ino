/*
    SD_Test

    SD card attached to SPI

    This example tests the "new architecture" (StorageImpl + FSImpl + unified FS API)
    through the recommended entrypoint: seeedfs::SdSpiFatFs.
*/

#include <Seeed_Arduino_FS.h>

#define LOG Serial

#ifdef WIO_LITE_AI
#error "This example is for SPI SD. For SDMMC on WIO_LITE_AI, use examples/SDMMC_Test."
#endif

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

static seeedfs::SdSpiFatFs g_fs;

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
            char child[260];
            if (strcmp(dirname, "/") == 0)
            {
                snprintf(child, sizeof(child), "/%s", name);
            }
            else
            {
                snprintf(child, sizeof(child), "%s/%s", dirname, name);
            }
            listDir(filesystem, child, depth + 1);
        }
        else
        {
            LOG.print("\t\t");
            LOG.print((uint32_t)dir.fileSize());
            LOG.println(" bytes");
        }
    }
}

static void createDir(seeedfs::FS &filesystem, const char *path)
{
    LOG.print("Creating Dir: ");
    LOG.println(path);
    if (filesystem.mkdir(path))
    {
        LOG.println("Dir created");
    }
    else
    {
        LOG.println("mkdir failed");
    }
}

static void removeDir(seeedfs::FS &filesystem, const char *path)
{
    LOG.print("Removing Dir: ");
    LOG.println(path);
    if (filesystem.rmdir(path))
    {
        LOG.println("Dir removed");
    }
    else
    {
        LOG.println("rmdir failed");
    }
}

static void readFile(seeedfs::FS &filesystem, const char *path)
{
    LOG.print("Reading file: ");
    LOG.println(path);

    seeedfs::File file = filesystem.open(path);
    if (!file)
    {
        LOG.println("Failed to open file for reading");
        return;
    }

    LOG.print("Read from file: ");
    while (file.available())
    {
        LOG.write(file.read());
    }
    file.close();
}

static void writeFile(seeedfs::FS &filesystem, const char *path, const char *message)
{
    LOG.print("Writing file: ");
    LOG.println(path);

    seeedfs::File file = filesystem.open(path, "w");
    if (!file)
    {
        LOG.println("Failed to open file for writing");
        return;
    }

    if (file.print(message))
    {
        LOG.println("File written");
    }
    else
    {
        LOG.println("Write failed");
    }
    file.close();
}

static void appendFile(seeedfs::FS &filesystem, const char *path, const char *message)
{
    LOG.print("Appending to file: ");
    LOG.println(path);

    seeedfs::File file = filesystem.open(path, "a");
    if (!file)
    {
        LOG.println("Failed to open file for appending");
        return;
    }
    if (file.print(message))
    {
        LOG.println("Message appended");
    }
    else
    {
        LOG.println("Append failed");
    }
    file.close();
}

static void renameFile(seeedfs::FS &filesystem, const char *path1, const char *path2)
{
    LOG.print("Renaming file ");
    LOG.print(path1);
    LOG.print(" to ");
    LOG.println(path2);
    if (filesystem.rename(path1, path2))
    {
        LOG.println("File renamed");
    }
    else
    {
        LOG.println("Rename failed");
    }
}

static void deleteFile(seeedfs::FS &filesystem, const char *path)
{
    LOG.print("Deleting file: ");
    LOG.println(path);
    if (filesystem.remove(path))
    {
        LOG.println("File deleted");
    }
    else
    {
        LOG.println("Delete failed");
    }
}

static void testFileIO(seeedfs::FS &filesystem, const char *path)
{
    seeedfs::File file = filesystem.open(path);
    if (!file)
    {
        LOG.println("Failed to open file for reading");
        return;
    }

    static uint8_t buf[512];
    size_t remaining = file.size();
    const size_t total = remaining;

    const uint32_t start = micros();
    while (remaining)
    {
        size_t toRead = remaining;
        if (toRead > sizeof(buf))
        {
            toRead = sizeof(buf);
        }
        file.read(buf, toRead);
        remaining -= toRead;
    }
    const uint32_t elapsedUs = micros() - start;
    file.close();

    LOG.print((uint32_t)total);
    LOG.print(" bytes read in ");
    LOG.print(elapsedUs);
    LOG.println(" us");
}

void setup()
{
    LOG.begin(115200);
    pinMode(5, OUTPUT);
    digitalWrite(5, HIGH);
    while (!LOG) {}

    delay(1000);

    LOG.println("\n===== Seeed_Arduino_FS SD_Test =====");
    LOG.print("CS = ");
    LOG.println(SDCARD_SS_PIN);
    LOG.print("SPI Hz = ");
    LOG.println((unsigned long)SDCARD_SPI_HZ);

    seeedfs::SdSpiCfg cfg;
    cfg.csPin = SDCARD_SS_PIN;
    cfg.spi = &SDCARD_SPI;
    cfg.spiHz = SDCARD_SPI_HZ;

#if defined(ARDUINO_ARCH_ESP32)
    cfg.sckPin = SDCARD_SCK_PIN;
    cfg.misoPin = SDCARD_MISO_PIN;
    cfg.mosiPin = SDCARD_MOSI_PIN;
#endif

    if (!g_fs.begin(cfg))
    {
        LOG.print("SD init failed: ");
        LOG.println(g_fs.lastErrorString());
        return;
    }

    seeedfs::FS &DEV = g_fs.fs();

    LOG.print("Storage: ");
    LOG.print(g_fs.type());
    LOG.print(", capacity: ");
    LOG.print((uint32_t)(g_fs.size() / (1024 * 1024)));
    LOG.println("MB");

    listDir(DEV, "/", 0);
    createDir(DEV, "/mydir");
    listDir(DEV, "/", 0);
    removeDir(DEV, "/mydir");
    listDir(DEV, "/", 1);
    writeFile(DEV, "/hello.txt", "Hello ");
    appendFile(DEV, "/hello.txt", "World!\n");
    readFile(DEV, "/hello.txt");
    deleteFile(DEV, "/foo.txt");
    renameFile(DEV, "/hello.txt", "/foo.txt");
    readFile(DEV, "/foo.txt");
    testFileIO(DEV, "/foo.txt");
    printFSInfo(DEV);
}

void loop() {}