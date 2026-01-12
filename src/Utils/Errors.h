#ifndef _ERROR_H_
#define _ERROR_H_

#include <stdint.h>

namespace seeedfs
{

    // General-purpose error codes for Seeed_Arduino_FS.
    // Keep values stable once released.
    enum SeeedFsError : int32_t
    {
        SEEED_FS_OK = 0, // OK

        // Generic
        SEEED_FS_ERR_UNKNOWN = -1, // Unknown

        // Config / wiring
        SEEED_FS_ERR_INVALID_CONFIG = 1, // Invalid config

        // Storage / driver
        SEEED_FS_ERR_DRIVER_NOT_SET = 100,          // Driver not set
        SEEED_FS_ERR_STORAGE_BEGIN_FAILED = 101,    // Storage begin failed
        SEEED_FS_ERR_STORAGE_NOT_READY = 102,       // Storage not ready
        SEEED_FS_ERR_UNSUPPORTED_SECTOR_SIZE = 103, // Unsupported sector size

        // SD (SPI)
        SEEED_FS_ERR_SD_INVALID_SPI = 200, // SD invalid SPI config
        SEEED_FS_ERR_SD_INIT_FAILED = 201, // SD init failed

        // SD HAL / protocol - Initialization
        SEEED_FS_ERR_SD_NOT_INITIALIZED = 232,         // SD not initialized
        SEEED_FS_ERR_SD_CMD0_FAILED = 220,             // SD CMD0 failed
        SEEED_FS_ERR_SD_CMD8_FAILED = 221,             // SD CMD8 failed
        SEEED_FS_ERR_SD_ACMD41_FAILED = 222,           // SD ACMD41 failed
        SEEED_FS_ERR_SD_CMD1_FAILED = 223,             // SD CMD1 failed
        SEEED_FS_ERR_SD_READ_OCR_FAILED = 224,         // SD READ_OCR failed
        SEEED_FS_ERR_SD_SET_BLOCKLEN_FAILED = 225,     // SD SET_BLOCKLEN failed
        SEEED_FS_ERR_SD_GET_SECTOR_COUNT_FAILED = 226, // SD get sector count failed
        SEEED_FS_ERR_SD_CSD_READ_FAILED = 250,         // SD CSD read failed
        SEEED_FS_ERR_SD_SEND_CSD_FAILED = 251,         // SD SEND_CSD failed

        // SD HAL / protocol - Read/Write Operations
        SEEED_FS_ERR_SD_READ_FAILED = 227,                      // SD read failed
        SEEED_FS_ERR_SD_WRITE_FAILED = 228,                     // SD write failed
        SEEED_FS_ERR_SD_CMD17_FAILED = 234,                     // SD CMD17 failed
        SEEED_FS_ERR_SD_CMD18_FAILED = 237,                     // SD CMD18 failed
        SEEED_FS_ERR_SD_CMD24_FAILED = 238,                     // SD CMD24 failed
        SEEED_FS_ERR_SD_CMD25_FAILED = 241,                     // SD CMD25 failed
        SEEED_FS_ERR_SD_INVALID_COUNT = 235,                    // SD invalid count
        SEEED_FS_ERR_SD_CMD12_FAILED = 236,                     // SD CMD12 failed
        SEEED_FS_ERR_SD_INVALID_TOKEN = 239,                    // SD invalid token
        SEEED_FS_ERR_SD_SEND_STATUS_FAILED = 240,               // SD SEND_STATUS failed
        SEEED_FS_ERR_SD_SET_WR_BLK_ERASE_COUNT_FAILED = 242,    // SD SET_WR_BLK_ERASE_COUNT failed
        SEEED_FS_ERR_SD_TIMEOUT_WAITING_WRITE_COMPLETION = 243, // SD timeout waiting write completion

        // SD HAL / protocol - Low-level Protocol
        SEEED_FS_ERR_SD_TIMEOUT = 229,                // SD timeout
        SEEED_FS_ERR_SD_CRC_ERROR = 230,              // SD CRC error
        SEEED_FS_ERR_SD_ILLEGAL_COMMAND = 231,        // SD illegal command
        SEEED_FS_ERR_SD_CARD_SELECTION_TIMEOUT = 233, // SD card selection timeout
        SEEED_FS_ERR_SD_WAIT_TIMEOUT = 244,           // SD wait timeout
        SEEED_FS_ERR_SD_READ_BYTES_TIMEOUT = 245,     // SD read bytes timeout
        SEEED_FS_ERR_SD_INVALID_DATA_TOKEN = 246,     // SD invalid data token
        SEEED_FS_ERR_SD_WRITE_BYTES_TIMEOUT = 247,    // SD write bytes timeout
        SEEED_FS_ERR_SD_TRANSACTION_TIMEOUT = 248,    // SD transaction timeout
        SEEED_FS_ERR_SD_APP_CMD_FAILED = 249,         // SD APP_CMD failed

        // FATFS layer
        SEEED_FS_ERR_FATFS_GET_DRIVE_FAILED = 300, // FATFS get drive failed
        SEEED_FS_ERR_FATFS_MOUNT_FAILED = 301,     // FATFS mount failed
        SEEED_FS_ERR_FATFS_UNMOUNT_FAILED = 302,   // FATFS unmount failed
        SEEED_FS_ERR_FATFS_NOT_MOUNTED = 303,      // FATFS not mounted
        SEEED_FS_ERR_FATFS_FORMAT_FAILED = 304,    // FATFS format failed
        SEEED_FS_ERR_FATFS_INFO_FAILED = 305,      // FATFS info failed
        SEEED_FS_ERR_FATFS_OPEN_FILE_FAILED = 306, // FATFS open file failed
        SEEED_FS_ERR_FATFS_STAT_FAILED = 307,      // FATFS stat failed
        SEEED_FS_ERR_FATFS_OPEN_DIR_FAILED = 308,  // FATFS open dir failed
        SEEED_FS_ERR_FATFS_RENAME_FAILED = 309,    // FATFS rename failed
        SEEED_FS_ERR_FATFS_REMOVE_FAILED = 310,    // FATFS remove failed
        SEEED_FS_ERR_FATFS_MKDIR_FAILED = 311,     // FATFS mkdir failed
    };

    typedef void (*SeeedFsErrorCallback)(int32_t code, int32_t detail);

    void seeedfs_setErrorCallback(SeeedFsErrorCallback cb);
    SeeedFsErrorCallback seeedfs_getErrorCallback(void);

    int32_t seeedfs_lastError(void);
    int32_t seeedfs_lastErrorDetail(void);
    void seeedfs_clearError(void);

    void seeedfs_reportError(int32_t code);
    void seeedfs_reportErrorDetail(int32_t code, int32_t detail);

} // namespace seeedfs

#endif