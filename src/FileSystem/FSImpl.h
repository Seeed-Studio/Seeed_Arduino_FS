// FSImpl.h
#ifndef _FS_IMPL_H_
#define _FS_IMPL_H_

#include <stddef.h>
#include <stdint.h>
#include "Storage/StorageImpl.h"

namespace seeedfs
{

    // Seek mode definitions
    enum SeekMode
    {
        SeekSet = 0,
        SeekCur = 1,
        SeekEnd = 2
    };

    // Abstract interface for file operations
    class FileImpl
    {
    public:
        virtual ~FileImpl() {}
        virtual size_t write(const uint8_t *buf, size_t size) = 0; // Write data
        virtual int read(uint8_t *buf, size_t size) = 0;           // Read data
        virtual void flush() = 0;                                  // Flush buffer
        virtual bool seek(uint32_t pos, SeekMode mode) = 0;        // Move file pointer
        virtual size_t position() const = 0;                       // Get current file pointer position
        virtual size_t size() const = 0;                           // Get file size
        virtual bool truncate(size_t size) = 0;                    // Truncate file to specified size
        virtual void close() = 0;                                  // Close file
        virtual const char *name() const = 0;                      // Get filename
        virtual bool isFile() const { return true; }               // Is it a file (default true)
        virtual bool isDirectory() const { return false; }         // Is it a directory (default false)
    };

    // Open modes
    enum OpenMode
    {
        OM_DEFAULT = 0, // Default mode
        OM_CREATE = 1,  // Create file (if not exists)
        OM_APPEND = 2,  // Append mode
        OM_TRUNCATE = 4 // Truncate mode
    };

    // Access modes
    enum AccessMode
    {
        AM_READ = 1,               // Read-only
        AM_WRITE = 2,              // Write-only
        AM_RW = AM_READ | AM_WRITE // Read-write
    };

    // Abstract interface for directory operations
    class DirImpl
    {
    public:
        virtual ~DirImpl() {}
        virtual FileImpl *openFile(OpenMode openMode, AccessMode accessMode) = 0; // Open file in directory
        virtual const char *fileName() = 0;                                       // Get current filename
        virtual size_t fileSize() = 0;                                            // Get current file size
        virtual bool isFile() const = 0;                                          // Is current entry a file
        virtual bool isDirectory() const = 0;                                     // Is current entry a directory
        virtual bool next() = 0;                                                  // Move to next entry
        virtual bool rewind() = 0;                                                // Reset to directory beginning
    };

    // Filesystem configuration struct
    struct FSConfig
    {
        bool autoFormat; // Auto format
        // Extensible for other config items
        FSConfig() : autoFormat(false) {}
    };

    // Filesystem info struct
    struct FSInfo
    {
        uint64_t totalBytes;   // Total capacity
        uint64_t usedBytes;    // Used capacity
        uint32_t blockSize;    // Block size
        uint32_t pageSize;     // Page size
        uint32_t maxOpenFiles; // Max open files
    };

    // Abstract interface for filesystem
    class FSImpl
    {
    public:
        virtual ~FSImpl() {}
        // Filesystem management
        virtual bool setConfig(const FSConfig &cfg) = 0; // Set filesystem config
        virtual bool begin() = 0;                        // Initialize filesystem
        virtual void end() = 0;                          // Close filesystem
        virtual bool format() = 0;                       // Format filesystem
        virtual bool info(FSInfo &info) = 0;             // Get filesystem info

        // File and directory operations (compatible with Seeed_FS interface)
        virtual FileImpl *open(const char *path, OpenMode openMode, AccessMode accessMode) = 0;
        virtual bool exists(const char *path) = 0;                         // Check if file/directory exists
        virtual DirImpl *openDir(const char *path) = 0;                    // Open directory
        virtual bool rename(const char *pathFrom, const char *pathTo) = 0; // Rename
        virtual bool remove(const char *path) = 0;                         // Remove file
        virtual bool mkdir(const char *path) = 0;                          // Create directory
        virtual bool rmdir(const char *path) = 0;                          // Remove directory

    protected:
        StorageImpl *_storage; // Associated storage device abstract interface

        FSImpl(StorageImpl *storage) : _storage(storage) {} // Constructor, bind storage device
        friend class FS;                                    // Allow FS class to access protected members
    };

} // namespace seeedfs

#endif // _FS_IMPL_H_