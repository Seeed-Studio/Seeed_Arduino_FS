/*
 * a wrapper for SDCard.
 * 
 * Created by Hongtai.liu 13 July 2019
 * 
 */

#include "Seeed_SD.h"
#include "Seeed_sdcard_hal.h"
#include <Seeed_FS.h>
#include "Arduino.h"
#ifdef KENDRYTE_K210
  #include <SPIClass.h>
#else
  #include <SPI.h>
#endif

extern SD_CardInfo cardinfo; //the cardinfo 

namespace fs {

boolean SDFS::begin(uint8_t ssPin, SPIClass &spi, int hz)
{
   card = &cardinfo;
   card->ssPin = ssPin; 
   card->spi = &spi;
   card->frequency = hz;
   spi.begin();

   FRESULT status = FR_OK;
   pinMode(card->ssPin, OUTPUT);
   status = f_mount(&root, _T("0:"), 1);
  
  if (status != FR_OK)
      return false;  
  else{ 
      return true;
  }
}

sdcard_type_t SDFS::cardType()
{
   return (sdcard_type_t)card->SD_cid.ProdRev;
}
uint64_t SDFS::cardSize()
{
    return card->CardCapacity;
}

uint64_t SDFS::totalBytes()
{
	FATFS* fsinfo;
	DWORD fre_clust;
	if(f_getfree("0:",&fre_clust,&fsinfo)!= 0) return 0;
    uint64_t size = ((uint64_t)(fsinfo->csize))*(fsinfo->n_fatent - 2)
#if _MAX_SS != 512
        *(fsinfo->ssize);
#else
        *512;
#endif
	return size;
}

uint64_t SDFS::usedBytes()
{
	FATFS* fsinfo;
	DWORD fre_clust;
	if(f_getfree("0:",&fre_clust,&fsinfo)!= 0) return 0;
	uint64_t size = ((uint64_t)(fsinfo->csize))*((fsinfo->n_fatent - 2) - (fsinfo->free_clst))
#if _MAX_SS != 512
        *(fsinfo->ssize);
#else
        *512;
#endif
	return size;
}

SDFS SD;
};
