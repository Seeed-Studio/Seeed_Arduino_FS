// StorageImpl.h
#ifndef _STORAGE_IMPL_H_
#define _STORAGE_IMPL_H_

#include <stddef.h>
#include <stdint.h>

// Forward declaration to avoid circular dependency
namespace seeedfs
{

    class StorageImpl
    {
    public:
        virtual ~StorageImpl() {}

        // Initialization and shutdown
        virtual bool begin() = 0;         // Initialize storage device
        virtual void end() = 0;           // Close storage device
        virtual bool isReady() const = 0; // Check if device is ready

        // Read/write operations (block level)
        virtual bool read(uint32_t sector, uint32_t offset, void *buffer, size_t size) = 0;        // Read data from specified sector
        virtual bool write(uint32_t sector, uint32_t offset, const void *buffer, size_t size) = 0; // Write data to specified sector
        virtual bool erase(uint32_t sector, size_t count) = 0;                                     // Erase specified sector (for flash devices)

        // Device information
        virtual uint64_t size() const = 0;        // Get total capacity (bytes)
        virtual uint32_t sectorSize() const = 0;  // Get sector size (bytes)
        virtual uint32_t sectorCount() const = 0; // Get total sector count
        virtual const char *type() const = 0;     // Get device type (e.g., "SD", "SPIFlash")

        // Optional capability: flush/sync (default no-op, returns success)
        virtual bool sync() { return true; }

    protected:
        StorageImpl() {}     // Protected constructor to prevent direct instantiation
        friend class FSImpl; // Allow FSImpl to access protected members
    };

} // namespace seeedfs

#endif // _STORAGE_IMPL_H_