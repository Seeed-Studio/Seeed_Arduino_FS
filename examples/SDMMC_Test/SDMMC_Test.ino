/*
  SDMMC_Test

  Demonstrates the new-architecture SDMMC path:
    StorageImpl (SdmmcStorage) + FSImpl (FatFs) + unified FS API.

  This example is only for WIO_LITE_AI.
*/

#include <Seeed_Arduino_FS.h>

#define LOG Serial

#ifndef WIO_LITE_AI
#error "This example requires WIO_LITE_AI (SDMMC)"
#endif

static seeedfs::SdmmcFatFs g_fs;

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

void setup()
{
    LOG.begin(115200);
    while (!LOG) {}

    LOG.println("\n===== Seeed_Arduino_FS SDMMC_Test (new architecture) =====");

    if (!g_fs.begin())
    {
        LOG.print("SDMMC init failed: ");
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

    seeedfs::File f = DEV.open("/sdmmc_hello.txt", "a");
    if (f)
    {
        f.println("Hello from SDMMC (StorageImpl + FATFS)");
        f.close();
        LOG.println("Write OK");
    }
    else
    {
        LOG.println("Open/write failed");
    }
}

void loop()
{
}
