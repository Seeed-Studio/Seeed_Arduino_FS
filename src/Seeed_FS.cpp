// Seeed_FS.cpp
#include "Seeed_FS.h"

#include "Utils/Errors.h"

namespace seeedfs
{
    // Static member initialization
    FSImpl *FS::_defaultImpl = nullptr;

    // File class implementation
    size_t File::write(uint8_t c)
    {
        return _impl ? _impl->write(&c, 1) : 0;
    }

    size_t File::write(const uint8_t *buf, size_t size)
    {
        return _impl ? _impl->write(buf, size) : 0;
    }

    int File::available()
    {
        return _impl ? (_impl->size() - _impl->position()) : 0;
    }

    int File::read()
    {
        if (!_impl)
            return -1;
        uint8_t result;
        return (_impl->read(&result, 1) == 1) ? result : -1;
    }

    int File::peek()
    {
        if (!_impl)
            return -1;
        size_t pos = _impl->position();
        int result = read();
        _impl->seek(pos, SeekSet);
        return result;
    }

    void File::flush()
    {
        if (_impl)
            _impl->flush();
    }

    int File::read(uint8_t *buf, size_t size)
    {
        return _impl ? _impl->read(buf, size) : 0;
    }

    bool File::seek(uint32_t pos, SeekMode mode)
    {
        return _impl ? _impl->seek(pos, mode) : false;
    }

    size_t File::position() const
    {
        return _impl ? _impl->position() : 0;
    }

    size_t File::size() const
    {
        return _impl ? _impl->size() : 0;
    }

    bool File::truncate(size_t size)
    {
        return _impl ? _impl->truncate(size) : false;
    }

    void File::close()
    {
        if (_impl)
        {
            _impl->close();
            delete _impl;
            _impl = nullptr;
        }
    }

    const char *File::name() const
    {
        return _impl ? _impl->name() : "";
    }

    bool File::isFile() const
    {
        return _impl ? _impl->isFile() : false;
    }

    bool File::isDirectory() const
    {
        return _impl ? _impl->isDirectory() : false;
    }

    // Dir class implementation
    File Dir::openFile(const char *mode)
    {
        if (!_impl)
            return File();
        OpenMode om = OM_DEFAULT;
        AccessMode am = AM_READ;
        if (mode[0] == 'w')
        {
            am = AM_WRITE;
            om = (OpenMode)(OM_CREATE | OM_TRUNCATE);
        }
        else if (mode[0] == 'a')
        {
            am = AM_WRITE;
            om = (OpenMode)(OM_CREATE | OM_APPEND);
        }
        return File(_impl->openFile(om, am));
    }

    const char *Dir::fileName()
    {
        return _impl ? _impl->fileName() : "";
    }

    size_t Dir::fileSize()
    {
        return _impl ? _impl->fileSize() : 0;
    }

    bool Dir::isFile() const
    {
        return _impl ? _impl->isFile() : false;
    }

    bool Dir::isDirectory() const
    {
        return _impl ? _impl->isDirectory() : false;
    }

    bool Dir::next()
    {
        return _impl ? _impl->next() : false;
    }

    bool Dir::rewind()
    {
        return _impl ? _impl->rewind() : false;
    }

    void Dir::close()
    {
        if (_impl)
        {
            delete _impl;
            _impl = nullptr;
        }
    }

    // FS class implementation
    bool FS::begin()
    {
        seeedfs_clearError();
        FSImpl *impl = _impl ? _impl : _defaultImpl;
        return impl ? impl->begin() : false;
    }

    void FS::end()
    {
        FSImpl *impl = _impl ? _impl : _defaultImpl;
        if (impl)
            impl->end();
    }

    bool FS::format()
    {
        seeedfs_clearError();
        FSImpl *impl = _impl ? _impl : _defaultImpl;
        return impl ? impl->format() : false;
    }

    bool FS::info(FSInfo &info)
    {
        seeedfs_clearError();
        FSImpl *impl = _impl ? _impl : _defaultImpl;
        return impl ? impl->info(info) : false;
    }

    bool FS::setConfig(const FSConfig &cfg)
    {
        seeedfs_clearError();
        FSImpl *impl = _impl ? _impl : _defaultImpl;
        return impl ? impl->setConfig(cfg) : false;
    }

    File FS::open(const char *path, const char *mode)
    {
        seeedfs_clearError();
        FSImpl *impl = _impl ? _impl : _defaultImpl;
        if (!impl)
            return File();
        OpenMode om = OM_DEFAULT;
        AccessMode am = AM_READ;
        if (mode[0] == 'w')
        {
            am = AM_WRITE;
            om = (OpenMode)(OM_CREATE | OM_TRUNCATE);
        }
        else if (mode[0] == 'a')
        {
            am = AM_WRITE;
            om = (OpenMode)(OM_CREATE | OM_APPEND);
        }
        return File(impl->open(path, om, am));
    }

    bool FS::exists(const char *path)
    {
        seeedfs_clearError();
        FSImpl *impl = _impl ? _impl : _defaultImpl;
        return impl ? impl->exists(path) : false;
    }

    Dir FS::openDir(const char *path)
    {
        seeedfs_clearError();
        FSImpl *impl = _impl ? _impl : _defaultImpl;
        return impl ? Dir(impl->openDir(path)) : Dir();
    }

    bool FS::remove(const char *path)
    {
        seeedfs_clearError();
        FSImpl *impl = _impl ? _impl : _defaultImpl;
        return impl ? impl->remove(path) : false;
    }

    bool FS::rename(const char *pathFrom, const char *pathTo)
    {
        seeedfs_clearError();
        FSImpl *impl = _impl ? _impl : _defaultImpl;
        return impl ? impl->rename(pathFrom, pathTo) : false;
    }

    bool FS::mkdir(const char *path)
    {
        seeedfs_clearError();
        FSImpl *impl = _impl ? _impl : _defaultImpl;
        return impl ? impl->mkdir(path) : false;
    }

    bool FS::rmdir(const char *path)
    {
        seeedfs_clearError();
        FSImpl *impl = _impl ? _impl : _defaultImpl;
        return impl ? impl->rmdir(path) : false;
    }
} // namespace seeedfs