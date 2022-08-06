/*
    Connect the SD card to the following pins:

    SD card attached to SPI or SDMMC

    modified 18 June 2021
    by Hongtai.liu

*/

#include <Seeed_Arduino_FS.h>


#ifdef WIO_LITE_AI
#define DEV  SDMMC 
#else
#define DEV  SD
#ifdef _SAMD21_
#define SDCARD_SS_PIN 1
#define SDCARD_SPI SPI
#endif 
#endif


#define LOG Serial

void listDir( fs::FS& fs, const char* dirname, int numTabs )
{
   File dir= fs.open(dirname);

   while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      LOG.print('\t');
    }
    LOG.print(entry.name());
    if (entry.isDirectory()) {
      LOG.println("/");
      listDir(fs,entry.name(), numTabs + 1);
    } else {
      // files have sizes, directories do not
      LOG.print("\t\t");
      LOG.print(entry.size(), DEC);
      LOG.println(" bytes");
    }
    entry.close();
  }
}

void createDir(fs::FS& fs, const char* path) {
    LOG.print("Creating Dir: ");
    LOG.println(path);
    if (fs.mkdir(path)) {
        LOG.println("Dir created");
    } else {
        LOG.println("mkdir failed");
    }
}

void removeDir(fs::FS& fs, const char* path) {
    LOG.print("Removing Dir: ");
    LOG.println(path);
    if (fs.rmdir(path)) {
        LOG.println("Dir removed");
    } else {
        LOG.println("rmdir failed");
    }
}

void readFile(fs::FS& fs, const char* path) {
    LOG.print("Reading Dir: ");
    LOG.println(path);
    File file = fs.open(path);
    if (!file) {
        LOG.println("Failed to open file for reading");
        return;
    }

    LOG.print("Read from file: ");
    while (file.available()) {
        LOG.write(file.read());
    }
    file.close();
}

void writeFile(fs::FS& fs, const char* path, const char* message) {
    LOG.print("Writing file: ");
    LOG.println(path);
    File file = fs.open(path, FILE_WRITE);
    if (!file) {
        LOG.println("Failed to open file for writing");
        return;
    }
    
    if (file.print(message)) {
        LOG.println("File written");
    } else {
        LOG.println("Write failed");
    }

    file.close();

}

void appendFile(fs::FS& fs, const char* path, const char* message) {
    LOG.print("Appending to file: ");
    LOG.println(path);

    File file = fs.open(path, FILE_APPEND);
    if (!file) {
        LOG.println("Failed to open file for appending");
        return;
    }
    if (file.print(message)) {
        LOG.println("Message appended");
    } else {
        LOG.println("Append failed");
    }
    file.close();
}

void renameFile(fs::FS& fs, const char* path1, const char* path2) {
    LOG.print("Renaming file ");
    LOG.print(path1);
    LOG.print(" to ");
    LOG.println(path2);
    if (fs.rename(path1, path2)) {
        LOG.println("File renamed");
    } else {
        LOG.println("Rename failed");
    }
}

void deleteFile(fs::FS& fs, const char* path) {
    LOG.print("Deleting file: ");
    LOG.println(path);
    if (fs.remove(path)) {
        LOG.println("File deleted");
    } else {
        LOG.println("Delete failed");
    }
}

void testFileIO(fs::FS& fs, const char* path) {
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = micros();
    uint32_t end = start;
    if (file) {
        len = file.size();
        size_t flen = len;
        start = micros();
        while (len) {
            size_t toRead = len;
            if (toRead > 512) {
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = micros() - start;
        LOG.print(flen);
        LOG.print(" bytes read for ");
        LOG.print(end);
        LOG.println(" ns");
        file.close();
    } else {
        LOG.println("Failed to open file for reading");
    }
}

void setup() {
    LOG.begin(115200);
    pinMode(5, OUTPUT);
    digitalWrite(5, HIGH);
    while (!LOG) {};

    delay(1000);

#ifdef WIO_LITE_AI
    while (!DEV.begin()) {
        LOG.println("Card Mount Failed");
        return;
    }  
#else
    while (!DEV.begin(SDCARD_SS_PIN,SDCARD_SPI,4000000UL)) {
        LOG.println("Card Mount Failed");
        return;
    }  
#endif


    uint8_t cardType = DEV.cardType();
    if (cardType == CARD_NONE) {
        LOG.println("No SD card attached");
        return;
    }

    uint32_t cardSize = DEV.cardSize() / (1024 * 1024);
    LOG.print("SD Card Size: ");
    LOG.print((uint32_t)cardSize);
    LOG.println("MB");


    listDir(DEV, "/", 0);
    createDir(DEV, "/mydir");
    listDir(DEV, "/", 0);
    removeDir(DEV, "/mydir");
    listDir(DEV, "/", 2);
    writeFile(DEV, "/hello.txt", "Hello ");
    appendFile(DEV, "/hello.txt", "World!\n");
    readFile(DEV, "/hello.txt");
    deleteFile(DEV, "/foo.txt");
    renameFile(DEV, "/hello.txt", "/foo.txt");
    readFile(DEV, "/foo.txt");
    testFileIO(DEV, "/foo.txt");
    uint32_t totalBytes = DEV.totalBytes();
    LOG.print("Total space: ");
    LOG.print(totalBytes / (1024 * 1024));
    LOG.println("MB");
    uint32_t usedBytes = DEV.usedBytes();
    LOG.print("Used space: ");
    uint32_t _usedBytes = usedBytes / (1024 * 1024);
    LOG.print(_usedBytes);
    LOG.println("MB");
}

void loop() {
    
}