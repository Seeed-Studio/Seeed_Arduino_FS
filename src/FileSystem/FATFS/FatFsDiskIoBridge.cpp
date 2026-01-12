#include "FileSystem/FATFS/FatFsDiskIoBridge.h"

namespace seeedfs
{

    StorageImpl *FatFsDiskIoBridge::_storages[_VOLUMES] = {nullptr};

    ff_diskio_impl_t FatFsDiskIoBridge::_diskio_impl = {
        .init = FatFsDiskIoBridge::disk_initialize,
        .status = FatFsDiskIoBridge::disk_status,
        .read = FatFsDiskIoBridge::disk_read,
        .write = FatFsDiskIoBridge::disk_write,
        .ioctl = FatFsDiskIoBridge::disk_ioctl,
    };

    void FatFsDiskIoBridge::attach(BYTE pdrv, StorageImpl *storage)
    {
        if (pdrv >= _VOLUMES)
        {
            return;
        }
        _storages[pdrv] = storage;
    }

    void FatFsDiskIoBridge::detach(BYTE pdrv)
    {
        if (pdrv >= _VOLUMES)
        {
            return;
        }
        _storages[pdrv] = nullptr;
    }

    StorageImpl *FatFsDiskIoBridge::storage(BYTE pdrv)
    {
        if (pdrv >= _VOLUMES)
        {
            return nullptr;
        }
        return _storages[pdrv];
    }

    const ff_diskio_impl_t *FatFsDiskIoBridge::diskioImpl()
    {
        return &_diskio_impl;
    }

    DSTATUS FatFsDiskIoBridge::disk_initialize(BYTE pdrv)
    {
        if (pdrv >= _VOLUMES || _storages[pdrv] == nullptr)
        {
            return STA_NOINIT;
        }
        return (_storages[pdrv]->begin() && _storages[pdrv]->isReady()) ? 0 : STA_NOINIT;
    }

    DSTATUS FatFsDiskIoBridge::disk_status(BYTE pdrv)
    {
        if (pdrv >= _VOLUMES || _storages[pdrv] == nullptr)
        {
            return STA_NOINIT;
        }
        return _storages[pdrv]->isReady() ? 0 : STA_NOINIT;
    }

    DRESULT FatFsDiskIoBridge::disk_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count)
    {
        if (pdrv >= _VOLUMES || _storages[pdrv] == nullptr)
        {
            return RES_NOTRDY;
        }

        const uint32_t ss = _storages[pdrv]->sectorSize();
        if (ss < _MIN_SS || ss > _MAX_SS)
        {
            return RES_PARERR;
        }

        for (UINT i = 0; i < count; i++)
        {
            if (!_storages[pdrv]->read(sector + i, 0, buff + (size_t)i * ss, ss))
            {
                return RES_ERROR;
            }
        }
        return RES_OK;
    }

    DRESULT FatFsDiskIoBridge::disk_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count)
    {
        if (pdrv >= _VOLUMES || _storages[pdrv] == nullptr)
        {
            return RES_NOTRDY;
        }

        const uint32_t ss = _storages[pdrv]->sectorSize();
        if (ss < _MIN_SS || ss > _MAX_SS)
        {
            return RES_PARERR;
        }

        for (UINT i = 0; i < count; i++)
        {
            if (!_storages[pdrv]->write(sector + i, 0, buff + (size_t)i * ss, ss))
            {
                return RES_ERROR;
            }
        }
        return RES_OK;
    }

    DRESULT FatFsDiskIoBridge::disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
    {
        if (pdrv >= _VOLUMES || _storages[pdrv] == nullptr)
        {
            return RES_PARERR;
        }

        switch (cmd)
        {
        case CTRL_SYNC:
            return _storages[pdrv]->sync() ? RES_OK : RES_ERROR;
        case GET_SECTOR_COUNT:
            *((DWORD *)buff) = _storages[pdrv]->sectorCount();
            return RES_OK;
        case GET_SECTOR_SIZE:
            *((WORD *)buff) = _storages[pdrv]->sectorSize();
            return RES_OK;
        case GET_BLOCK_SIZE:
            *((DWORD *)buff) = 1;
            return RES_OK;
        default:
            return RES_PARERR;
        }
    }

} // namespace seeedfs
