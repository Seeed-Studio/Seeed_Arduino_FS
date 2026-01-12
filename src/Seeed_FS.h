// Seeed_FS.h
#ifndef _SEEED_FS_H_
#define _SEEED_FS_H_

#include <Arduino.h>
#include "FileSystem/FSImpl.h" // Include abstract filesystem layer interface

namespace seeedfs
{

    // Forward declarations
    class FileImpl;
    class DirImpl;
    class FSImpl;

    // File class
    class File : public Stream
    {
    public:
        File(FileImpl *impl = nullptr) : _impl(impl) {}
        ~File() { close(); }

        // Print interface
        size_t write(uint8_t c) override;
        size_t write(const uint8_t *buf, size_t size) override;

        // Stream interface
        int available() override;
        int read() override;
        int peek() override;
        void flush() override;
        size_t readBytes(char *buffer, size_t length) { return read((uint8_t *)buffer, length); }
        int read(uint8_t *buf, size_t size);
        bool seek(uint32_t pos, SeekMode mode);
        bool seek(uint32_t pos) { return seek(pos, SeekSet); }
        size_t position() const;
        size_t size() const;
        bool truncate(size_t size);
        void close();
        operator bool() const { return _impl != nullptr; }
        const char *name() const;
        bool isFile() const;
        bool isDirectory() const;

    private:
        FileImpl *_impl; // File implementation pointer
    };

    // Directory class
    class Dir
    {
    public:
        Dir(DirImpl *impl = nullptr) : _impl(impl) {}
        ~Dir() { close(); }

        File openFile(const char *mode);
        const char *fileName();
        size_t fileSize();
        bool isFile() const;
        bool isDirectory() const;
        bool next();
        bool rewind();
        bool rewindDirectory() { return rewind(); }

    private:
        void close();
        DirImpl *_impl; // Directory implementation pointer
    };

    // Filesystem class
    class FS
    {
    public:
        // Option 1: Specify FSImpl in constructor
        FS(FSImpl *impl = nullptr) : _impl(impl) {}

        // Option 2: Global configuration function
        static void begin(FSImpl *impl) { _defaultImpl = impl; }

        // Filesystem management
        bool begin();
        void end();
        bool format();
        bool info(FSInfo &info);
        bool setConfig(const FSConfig &cfg);

        // File and directory operations
        File open(const char *path, const char *mode = "r");
        bool exists(const char *path);
        Dir openDir(const char *path);
        bool remove(const char *path);
        bool rename(const char *pathFrom, const char *pathTo);
        bool mkdir(const char *path);
        bool rmdir(const char *path);

    private:
        FSImpl *_impl;               // Instance-specific filesystem implementation
        static FSImpl *_defaultImpl; // Global default filesystem implementation
    };

} // namespace seeedfs

// Optional: Import types into global namespace (disabled by default to avoid conflicts with ESP32 Core FS/SD libraries)
#ifdef SEEEDFS_ENABLE_GLOBALS
using seeedfs::Dir;
using seeedfs::File;
using seeedfs::FS;
using seeedfs::FSConfig;
using seeedfs::FSInfo;
using seeedfs::SeekCur;
using seeedfs::SeekEnd;
using seeedfs::SeekMode;
using seeedfs::SeekSet;
#endif

#endif // _SEEED_FS_H_