#ifndef __SEEED_FILE__
#define __SEEED_FILE__
#include "fatfs/ff.h"

class File : public Stream {
 private:
  char _name[_MAX_LFN+2]; // file name
  FIL *_file;  // underlying file pointer
  DIR *_dir;  // if open a dir

public:
  File(FIL f, const char *name);     // wraps an underlying SdFile
  File(DIR d, const char *name);
  File(void);      // 'empty' constructor
  virtual size_t write(uint8_t);
  virtual size_t write(const uint8_t *buf, size_t size);
  virtual int read();
  virtual int peek();
  virtual int available();
  virtual void flush();
  long read(void *buf, uint32_t nbyte);
  boolean seek(uint32_t pos);
  uint32_t position();
  uint32_t size();
  void close();
  operator bool();
  char * name();

  boolean isDirectory(void);
  File openNextFile(uint8_t mode = FA_READ);
  void rewindDirectory(void);
  
  using Print::write;
};
#endif