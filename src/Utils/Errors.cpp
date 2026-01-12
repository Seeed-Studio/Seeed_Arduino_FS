#include "Errors.h"

namespace seeedfs
{

    static SeeedFsErrorCallback g_errorCallback = nullptr;
    static int32_t g_lastError = SEEED_FS_OK;
    static int32_t g_lastErrorDetail = 0;

    void seeedfs_setErrorCallback(SeeedFsErrorCallback cb)
    {
        g_errorCallback = cb;
    }

    SeeedFsErrorCallback seeedfs_getErrorCallback(void)
    {
        return g_errorCallback;
    }

    int32_t seeedfs_lastError(void)
    {
        return g_lastError;
    }

    int32_t seeedfs_lastErrorDetail(void)
    {
        return g_lastErrorDetail;
    }

    void seeedfs_clearError(void)
    {
        g_lastError = SEEED_FS_OK;
        g_lastErrorDetail = 0;
    }

    void seeedfs_reportError(int32_t code)
    {
        seeedfs_reportErrorDetail(code, 0);
    }

    void seeedfs_reportErrorDetail(int32_t code, int32_t detail)
    {
        g_lastError = code;
        g_lastErrorDetail = detail;

        if (g_errorCallback)
        {
            g_errorCallback(code, detail);
        }
    }

} // namespace seeedfs