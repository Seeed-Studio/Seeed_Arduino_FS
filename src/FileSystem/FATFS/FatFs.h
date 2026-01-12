#ifndef _SEEED_FATFS_H_
#define _SEEED_FATFS_H_

#include "FileSystem/FSImpl.h"
#include "FileSystem/FATFS/FatFsDiskIoBridge.h"
#include "fatfs/ff.h"
#include "fatfs/diskio.h"

namespace seeedfs
{

    class FATFS_File : public FileImpl
    {
    public:
        FATFS_File(FIL *file, const char *name);
        ~FATFS_File();
        size_t write(const uint8_t *buf, size_t size) override;
        int read(uint8_t *buf, size_t size) override;
        void flush() override;
        bool seek(uint32_t pos, SeekMode mode) override;
        size_t position() const override;
        size_t size() const override;
        bool truncate(size_t size) override;
        void close() override;
        const char *name() const override;

    private:
        FIL *_file;
        char _name[_MAX_LFN + 1];
    };

    class FATFS_Dir : public DirImpl
    {
    public:
        FATFS_Dir(DIR *dir, const char *path);
        ~FATFS_Dir();
        FileImpl *openFile(OpenMode openMode, AccessMode accessMode) override;
        const char *fileName() override;
        size_t fileSize() override;
        bool isFile() const override;
        bool isDirectory() const override;
        bool next() override;
        bool rewind() override;

    private:
        DIR *_dir;
        FILINFO _fno;
        char _path[_MAX_LFN + 1];
        char _fname[_MAX_LFN + 1];
    };

    class FatFs : public FSImpl
    {
    public:
        // Type definition: time callback function
        // Should return timestamp in FATFS format (DWORD)
        // bit31:25 Year origin from 1980 (0..127)
        // bit24:21 Month (1..12)
        // bit20:16 Day (1..31)
        // bit15:11 Hour (0..23)
        // bit10:5  Minute (0..59)
        // bit4:0   Second / 2 (0..29)
        typedef DWORD (*TimeCallback)();

        FatFs(StorageImpl *storage);
        ~FatFs();

        // Register time callback function
        static void registerTimeCallback(TimeCallback cb);

        // FSImpl interface implementation
        bool setConfig(const FSConfig &cfg) override;
        bool begin() override;
        void end() override;
        bool format() override;
        bool info(FSInfo &info) override;
        FileImpl *open(const char *path, OpenMode openMode, AccessMode accessMode) override;
        bool exists(const char *path) override;
        DirImpl *openDir(const char *path) override;
        bool rename(const char *pathFrom, const char *pathTo) override;
        bool remove(const char *path) override;
        bool mkdir(const char *path) override;
        bool rmdir(const char *path) override;

    public:
        // Internal accessor for get_fattime call
        static TimeCallback _timeCallback;

    private:
        FATFS _fs;
        bool _mounted;
        TCHAR _drive[3];
        BYTE _pdrv;
    };

} // namespace seeedfs

#endif // _SEEED_FATFS_H_