#pragma once

#include "Storage/StorageImpl.h"

#include "fatfs/ff.h"
#include "fatfs/diskio.h"

namespace seeedfs
{

    class FatFsDiskIoBridge
    {
    public:
        static void attach(BYTE pdrv, StorageImpl *storage);
        static void detach(BYTE pdrv);
        static StorageImpl *storage(BYTE pdrv);

        static const ff_diskio_impl_t *diskioImpl();

    private:
        static DSTATUS disk_initialize(BYTE pdrv);
        static DSTATUS disk_status(BYTE pdrv);
        static DRESULT disk_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count);
        static DRESULT disk_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count);
        static DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff);

    private:
        static StorageImpl *_storages[_VOLUMES];
        static ff_diskio_impl_t _diskio_impl;
    };

} // namespace seeedfs
