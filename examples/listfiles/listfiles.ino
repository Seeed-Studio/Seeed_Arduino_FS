/*
    Listfiles

    This example shows how print out the files in a
    directory on a SD card

    The circuit:
    SD card attached to SPI bus or SDMMC


    created   Nov 2010
    by David A. Mellis
    modified 9 Apr 2012
    by Tom Igoe
    modified 2 Feb 2014
    by Scott Fitzgerald
    modified 18 July 2021
    by Hongtai.liu

    This example code is in the public domain.

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
#define SDCARD_SPI_HZ 400000UL
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

#ifndef LED_BUILTIN
#define LED_BUILTIN D0
#endif

#ifdef WIO_LITE_AI
static seeedfs::SdmmcFatFs g_fs;
#else
static seeedfs::SdSpiFatFs g_fs;
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

    LOG.print(dir.fileName());
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

void setup() {
  // Open LOG communications and wait for port to open:
  pinMode(LED_BUILTIN, OUTPUT);
  LOG.begin(115200);
  while (!LOG) {
    ; // wait for LOG port to connect. Needed for native USB port only
  }

  LOG.println("Initializing SD card...");

#ifdef WIO_LITE_AI
  if (!g_fs.begin()) {
    LOG.print("SDMMC init failed: ");
    LOG.println(g_fs.lastErrorString());
    return;
  }
#else
  seeedfs::SdSpiCfg cfg;
  cfg.csPin = SDCARD_SS_PIN;
  cfg.spi = &SDCARD_SPI;
  cfg.spiHz = SDCARD_SPI_HZ;
#if defined(ARDUINO_ARCH_ESP32)
  cfg.sckPin = SDCARD_SCK_PIN;
  cfg.misoPin = SDCARD_MISO_PIN;
  cfg.mosiPin = SDCARD_MOSI_PIN;
#endif
  if (!g_fs.begin(cfg)) {
    LOG.print("SD init failed: ");
    LOG.println(g_fs.lastErrorString());
    return;
  }
#endif

  seeedfs::FS &DEV = g_fs.fs();

  LOG.println("initialization done.");

  listDir(DEV, "/", 0);

  LOG.println("done!");
}

void loop() {
  // nothing happens after setup finishes.
    digitalWrite(LED_BUILTIN, LOW);  
    delay(50);                    
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);   
}

// legacy openNextFile-based listing removed; use seeedfs::Dir instead.