#include "FatFs.h"
#include "FileSystem/FATFS/FatFsDiskIoBridge.h"
#include "Utils/Errors.h"
#include <string.h>
#include <stdio.h>

extern "C"
{
    DWORD get_fattime(void);
}

namespace seeedfs
{
    // Static member initialization
    FatFs::TimeCallback FatFs::_timeCallback = nullptr;

    void FatFs::registerTimeCallback(TimeCallback cb)
    {
        _timeCallback = cb;
    }
}

// Global function get_fattime implementation, for FatFs low-level call
DWORD get_fattime(void)
{
    if (seeedfs::FatFs::_timeCallback)
    {
        return seeedfs::FatFs::_timeCallback();
    }
    // Default return a fixed timestamp, e.g., 2026/01/01 00:00:00
    // ((DWORD)(2026 - 1980) << 25 | (DWORD)1 << 21 | (DWORD)1 << 16)
    return ((DWORD)(2026 - 1980) << 25 | (DWORD)1 << 21 | (DWORD)1 << 16);
}

namespace
{

    static void narrow_from_tchar(const TCHAR *in, char *out, size_t outLen)
    {
        if (!out || outLen == 0)
        {
            return;
        }

        if (!in)
        {
            out[0] = '\0';
            return;
        }

#if _LFN_UNICODE
        size_t i = 0;
        for (; i + 1 < outLen && in[i] != 0; i++)
        {
            const uint32_t ch = (uint32_t)in[i];
            out[i] = (ch <= 0x7FU) ? (char)ch : '?';
        }
        out[i] = '\0';
#else
        strncpy(out, (const char *)in, outLen);
        out[outLen - 1] = '\0';
#endif
    }

    static void tchar_from_narrow(const char *in, TCHAR *out, size_t outLen)
    {
        if (!out || outLen == 0)
        {
            return;
        }

        if (!in)
        {
            out[0] = 0;
            return;
        }

#if _LFN_UNICODE
        size_t i = 0;
        for (; i + 1 < outLen && in[i] != '\0'; i++)
        {
            out[i] = (TCHAR)(uint8_t)in[i];
        }
        out[i] = 0;
#else
        strncpy((char *)out, in, outLen);
        ((char *)out)[outLen - 1] = '\0';
#endif
    }

} // namespace

namespace seeedfs
{

    // FATFS_File implementation
    FATFS_File::FATFS_File(FIL *file, const char *name) : _file(file)
    {
        strncpy(_name, name, _MAX_LFN);
        _name[_MAX_LFN] = '\0';
    }

    FATFS_File::~FATFS_File()
    {
        close();
    }

    size_t FATFS_File::write(const uint8_t *buf, size_t size)
    {
        UINT bw;
        return (f_write(_file, buf, size, &bw) == FR_OK) ? bw : 0;
    }

    int FATFS_File::read(uint8_t *buf, size_t size)
    {
        UINT br;
        return (f_read(_file, buf, size, &br) == FR_OK) ? br : -1;
    }

    void FATFS_File::flush()
    {
        f_sync(_file);
    }

    bool FATFS_File::seek(uint32_t pos, SeekMode mode)
    {
        switch (mode)
        {
        case SeekSet:
            return f_lseek(_file, pos) == FR_OK;
        case SeekCur:
            return f_lseek(_file, f_tell(_file) + pos) == FR_OK;
        case SeekEnd:
            return f_lseek(_file, f_size(_file) - pos) == FR_OK;
        default:
            return false;
        }
    }

    size_t FATFS_File::position() const
    {
        return f_tell(_file);
    }

    size_t FATFS_File::size() const
    {
        return f_size(_file);
    }

    bool FATFS_File::truncate(size_t size)
    {
        DWORD currentPos = f_tell(_file);
        if (f_lseek(_file, size) != FR_OK)
        {
            return false;
        }

        if (f_truncate(_file) != FR_OK)
        {
            f_lseek(_file, currentPos);
            return false;
        }

        if (currentPos < size)
        {
            f_lseek(_file, currentPos);
        }

        return true;
    }

    void FATFS_File::close()
    {
        if (_file)
        {
            f_close(_file);
            delete _file;
            _file = nullptr;
        }
    }

    const char *FATFS_File::name() const
    {
        const char *p = strrchr(_name, '/');
        return p ? p + 1 : _name;
    }

    // FATFS_Dir implementation
    FATFS_Dir::FATFS_Dir(DIR *dir, const char *path) : _dir(dir)
    {
        strncpy(_path, path, _MAX_LFN);
        _path[_MAX_LFN] = '\0';
        _fname[0] = '\0';
    }

    FATFS_Dir::~FATFS_Dir()
    {
        if (_dir)
        {
            f_closedir(_dir);
            delete _dir;
            _dir = nullptr;
        }
    }

    FileImpl *FATFS_Dir::openFile(OpenMode openMode, AccessMode accessMode)
    {
        BYTE mode = 0;
        if (accessMode & AM_READ)
            mode |= FA_READ;
        if (accessMode & AM_WRITE)
            mode |= FA_WRITE;
        if (openMode & OM_CREATE)
            mode |= FA_CREATE_ALWAYS;
        if (openMode & OM_APPEND)
            mode |= FA_OPEN_APPEND;

        char fullPath[_MAX_LFN + 1];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", _path, _fname);

        TCHAR tFullPath[_MAX_LFN + 1];
        tchar_from_narrow(fullPath, tFullPath, sizeof(tFullPath) / sizeof(tFullPath[0]));

        FIL *file = new FIL;
        if (f_open(file, tFullPath, mode) == FR_OK)
        {
            return new FATFS_File(file, _fname);
        }
        delete file;
        return nullptr;
    }

    const char *FATFS_Dir::fileName()
    {
        return _fname;
    }

    size_t FATFS_Dir::fileSize()
    {
        return _fno.fsize;
    }

    bool FATFS_Dir::isFile() const
    {
        return !(_fno.fattrib & AM_DIR);
    }

    bool FATFS_Dir::isDirectory() const
    {
        return _fno.fattrib & AM_DIR;
    }

    bool FATFS_Dir::next()
    {
        if (f_readdir(_dir, &_fno) != FR_OK || _fno.fname[0] == 0)
        {
            _fname[0] = '\0';
            return false;
        }
        narrow_from_tchar(_fno.fname, _fname, sizeof(_fname));
        return true;
    }

    bool FATFS_Dir::rewind()
    {
        return f_rewinddir(_dir) == FR_OK;
    }

    // FatFs implementation
    FatFs::FatFs(StorageImpl *storage) : FSImpl(storage), _mounted(false), _pdrv(0xFF)
    {
        _drive[0] = _T('0');
        _drive[1] = _T(':');
        _drive[2] = 0;
    }

    FatFs::~FatFs()
    {
        end();
    }

    bool FatFs::setConfig(const FSConfig &cfg)
    {
        return true;
    }

    bool FatFs::begin()
    {
        if (_mounted)
            return true;

        if (ff_diskio_get_drive(&_pdrv) != 0 || _pdrv == 0xFF)
        {
            seeedfs_reportError(SEEED_FS_ERR_FATFS_GET_DRIVE_FAILED);
            return false;
        }

        FatFsDiskIoBridge::attach(_pdrv, _storage);
        ff_diskio_register(_pdrv, FatFsDiskIoBridge::diskioImpl());
        _drive[0] = _T('0' + _pdrv);

        FRESULT res = f_mount(&_fs, _drive, 1);
        if (res != FR_OK)
        {
            seeedfs_reportErrorDetail(SEEED_FS_ERR_FATFS_MOUNT_FAILED, (int32_t)res);

            ff_diskio_register(_pdrv, nullptr);
            FatFsDiskIoBridge::detach(_pdrv);
            return false;
        }
        _mounted = true;
        return true;
    }

    void FatFs::end()
    {
        if (_mounted)
        {
            FRESULT res = f_mount(nullptr, _drive, 0);
            if (res != FR_OK)
            {
                seeedfs_reportErrorDetail(SEEED_FS_ERR_FATFS_UNMOUNT_FAILED, (int32_t)res);
            }
            ff_diskio_register(_pdrv, nullptr);
            FatFsDiskIoBridge::detach(_pdrv);
            _mounted = false;
        }
        if (_storage)
        {
            _storage->end();
        }
    }

    bool FatFs::format()
    {
        if (!_mounted)
        {
            seeedfs_reportError(SEEED_FS_ERR_FATFS_NOT_MOUNTED);
            return false;
        }
        static uint8_t work[_MAX_SS];
        FRESULT res = f_mkfs(_drive, FM_FAT32, 0, work, sizeof(work));
        if (res != FR_OK)
        {
            seeedfs_reportErrorDetail(SEEED_FS_ERR_FATFS_FORMAT_FAILED, (int32_t)res);
            return false;
        }
        // Remount
        f_mount(nullptr, _drive, 0);
        if (res != FR_OK)
        {
            seeedfs_reportErrorDetail(SEEED_FS_ERR_FATFS_UNMOUNT_FAILED, (int32_t)res);
        }
        res = f_mount(&_fs, _drive, 1);
        if (res != FR_OK)
        {
            seeedfs_reportErrorDetail(SEEED_FS_ERR_FATFS_MOUNT_FAILED, (int32_t)res);
            return false;
        }
        return res == FR_OK;
    }

    bool FatFs::info(FSInfo &info)
    {
        DWORD free_clusters;
        FATFS *fs_ptr = &_fs;
        FRESULT res = f_getfree(_drive, &free_clusters, &fs_ptr);
        if (res != FR_OK)
        {
            seeedfs_reportErrorDetail(SEEED_FS_ERR_FATFS_INFO_FAILED, (int32_t)res);
            return false;
        }
        uint32_t sectorSize = 512;
#if _MAX_SS != _MIN_SS
        sectorSize = _fs.ssize;
#endif
        info.totalBytes = (uint64_t)(_fs.n_fatent - 2) * _fs.csize * sectorSize;
        info.usedBytes = info.totalBytes - (uint64_t)free_clusters * _fs.csize * sectorSize;
        info.blockSize = sectorSize;
        info.pageSize = sectorSize;
        info.maxOpenFiles = _FS_TINY ? 1 : _MAX_SS / 512;
        return true;
    }

    FileImpl *FatFs::open(const char *path, OpenMode openMode, AccessMode accessMode)
    {
        BYTE mode = 0;
        if (accessMode & AM_READ)
            mode |= FA_READ;
        if (accessMode & AM_WRITE)
            mode |= FA_WRITE;

        if (openMode & OM_TRUNCATE) 
        {
            mode |= FA_CREATE_ALWAYS;
        }
        else if (openMode & OM_APPEND) 
        {
            mode |= FA_OPEN_APPEND;
        }
        else if (openMode & OM_CREATE) 
        {
            mode |= FA_OPEN_ALWAYS;
        }

        char fullPath[_MAX_LFN + 1];
        const char *p = path;
        if (p[0] == '/')
            p++; // Skip leading slash
        snprintf(fullPath, sizeof(fullPath), "%u:/%s", (unsigned)_pdrv, p);

        TCHAR tFullPath[_MAX_LFN + 1];
        tchar_from_narrow(fullPath, tFullPath, sizeof(tFullPath) / sizeof(tFullPath[0]));

        FIL *file = new FIL;
        FRESULT res = f_open(file, tFullPath, mode);
        if (res == FR_OK)
        {
            return new FATFS_File(file, path);
        }
        seeedfs_reportErrorDetail(SEEED_FS_ERR_FATFS_OPEN_FILE_FAILED, (int32_t)res);
        delete file;
        return nullptr;
    }

    bool FatFs::exists(const char *path)
    {
        char fullPath[_MAX_LFN + 1];
        const char *p = path;
        if (p[0] == '/')
            p++;
        snprintf(fullPath, sizeof(fullPath), "%u:/%s", (unsigned)_pdrv, p);

        TCHAR tFullPath[_MAX_LFN + 1];
        tchar_from_narrow(fullPath, tFullPath, sizeof(tFullPath) / sizeof(tFullPath[0]));
        FILINFO fno;
        FRESULT res = f_stat(tFullPath, &fno);
        if (res != FR_OK)
        {
            seeedfs_reportErrorDetail(SEEED_FS_ERR_FATFS_STAT_FAILED, (int32_t)res);
            return false;
        }
        return true;
    }

    DirImpl *FatFs::openDir(const char *path)
    {
        char fullPath[_MAX_LFN + 1];
        const char *p = path;
        if (p[0] == '/')
            p++; // Skip leading slash
        snprintf(fullPath, sizeof(fullPath), "%u:/%s", (unsigned)_pdrv, p);

        TCHAR tFullPath[_MAX_LFN + 1];
        tchar_from_narrow(fullPath, tFullPath, sizeof(tFullPath) / sizeof(tFullPath[0]));

        DIR *dir = new DIR;
        FRESULT res = f_opendir(dir, tFullPath);
        if (res == FR_OK)
        {
            return new FATFS_Dir(dir, fullPath);
        }
        seeedfs_reportErrorDetail(SEEED_FS_ERR_FATFS_OPEN_DIR_FAILED, (int32_t)res);
        delete dir;
        return nullptr;
    }

    bool FatFs::rename(const char *pathFrom, const char *pathTo)
    {
        char fullPathFrom[_MAX_LFN + 1];
        char fullPathTo[_MAX_LFN + 1];
        const char *pFrom = pathFrom;
        const char *pTo = pathTo;
        if (pFrom[0] == '/')
            pFrom++;
        if (pTo[0] == '/')
            pTo++;
        snprintf(fullPathFrom, sizeof(fullPathFrom), "%u:/%s", (unsigned)_pdrv, pFrom);
        snprintf(fullPathTo, sizeof(fullPathTo), "%u:/%s", (unsigned)_pdrv, pTo);

        TCHAR tFrom[_MAX_LFN + 1];
        TCHAR tTo[_MAX_LFN + 1];
        tchar_from_narrow(fullPathFrom, tFrom, sizeof(tFrom) / sizeof(tFrom[0]));
        tchar_from_narrow(fullPathTo, tTo, sizeof(tTo) / sizeof(tTo[0]));

        FRESULT res = f_rename(tFrom, tTo);
        if (res != FR_OK)
        {
            seeedfs_reportErrorDetail(SEEED_FS_ERR_FATFS_RENAME_FAILED, (int32_t)res);
            return false;
        }
        return true;
    }

    bool FatFs::remove(const char *path)
    {
        char fullPath[_MAX_LFN + 1];
        const char *p = path;
        if (p[0] == '/')
            p++;
        snprintf(fullPath, sizeof(fullPath), "%u:/%s", (unsigned)_pdrv, p);

        TCHAR tFullPath[_MAX_LFN + 1];
        tchar_from_narrow(fullPath, tFullPath, sizeof(tFullPath) / sizeof(tFullPath[0]));
        FRESULT res = f_unlink(tFullPath);
        if (res != FR_OK)
        {
            seeedfs_reportErrorDetail(SEEED_FS_ERR_FATFS_REMOVE_FAILED, (int32_t)res);
            return false;
        }
        return true;
    }

    bool FatFs::mkdir(const char *path)
    {
        char fullPath[_MAX_LFN + 1];
        const char *p = path;
        if (p[0] == '/')
            p++;
        snprintf(fullPath, sizeof(fullPath), "%u:/%s", (unsigned)_pdrv, p);

        TCHAR tFullPath[_MAX_LFN + 1];
        tchar_from_narrow(fullPath, tFullPath, sizeof(tFullPath) / sizeof(tFullPath[0]));
        FRESULT res = f_mkdir(tFullPath);
        if (res != FR_OK)
        {
            seeedfs_reportErrorDetail(SEEED_FS_ERR_FATFS_MKDIR_FAILED, (int32_t)res);
            return false;
        }
        return true;
    }

    bool FatFs::rmdir(const char *path)
    {
        return remove(path); // FATFS uses unlink to remove directory
    }

} // namespace seeedfs