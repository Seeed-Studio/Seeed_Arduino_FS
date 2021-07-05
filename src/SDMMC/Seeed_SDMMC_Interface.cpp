#ifdef WIO_LITE_AI

#include "Seeed_SDMMC_Interface.h"
#include "stm32h7xx_hal.h"
#include "../fatfs/diskio.h"
#include "../fatfs/ffconf.h"
#include "../fatfs/ff.h"

/* SD status structure definition */
#define   MSD_OK                        ((uint8_t)0)
#define   MSD_ERROR                     ((uint8_t)1)
#define   MSD_ERROR_SD_NOT_PRESENT      ((uint8_t)2)

/* Common Error codes */
#define BSP_ERROR_NONE                    0
#define BSP_ERROR_NO_INIT                -1
#define BSP_ERROR_WRONG_PARAM            -2
#define BSP_ERROR_BUSY                   -3
#define BSP_ERROR_PERIPH_FAILURE         -4
#define BSP_ERROR_COMPONENT_FAILURE      -5
#define BSP_ERROR_UNKNOWN_FAILURE        -6
#define BSP_ERROR_UNKNOWN_COMPONENT      -7
#define BSP_ERROR_BUS_FAILURE            -8
#define BSP_ERROR_CLOCK_FAILURE          -9
#define BSP_ERROR_MSP_FAILURE            -10
#define BSP_ERROR_FEATURE_NOT_SUPPORTED  -11

/* BSP OSPI error codes */
#define BSP_ERROR_OSPI_SUSPENDED          -20
#define BSP_ERROR_OSPI_ASSIGN_FAILURE     -24
#define BSP_ERROR_OSPI_SETUP_FAILURE      -25
#define BSP_ERROR_OSPI_MMP_LOCK_FAILURE   -26
#define BSP_ERROR_OSPI_MMP_UNLOCK_FAILURE -27

/* BSP TS error code */
#define BSP_ERROR_TS_TOUCH_NOT_DETECTED   -30

/* BSP BUS error codes */
#define BSP_ERROR_BUS_TRANSACTION_FAILURE    -100
#define BSP_ERROR_BUS_ARBITRATION_LOSS       -101
#define BSP_ERROR_BUS_ACKNOWLEDGE_FAILURE    -102
#define BSP_ERROR_BUS_PROTOCOL_FAILURE       -103

#define BSP_ERROR_BUS_MODE_FAULT             -104
#define BSP_ERROR_BUS_FRAME_ERROR            -105
#define BSP_ERROR_BUS_CRC_ERROR              -106
#define BSP_ERROR_BUS_DMA_FAILURE            -107

#define SD_INSTANCES_NBR                        2UL


#define SD_TIMEOUT                              3 * 1000
#define SD_DEFAULT_BLOCK_SIZE                   512

#ifndef SD_WRITE_TIMEOUT
#define SD_WRITE_TIMEOUT         100U
#endif

#ifndef SD_READ_TIMEOUT
#define SD_READ_TIMEOUT          100U
#endif

/**
  * @brief  SD transfer state definition
  */
#define   SD_TRANSFER_OK         0U
#define   SD_TRANSFER_BUSY       1U

/**
  * @brief SD-detect signal
  */
#define SD_PRESENT               1U
#define SD_NOT_PRESENT           0U

#define SECTORSIZE 4096
#define ENABLE_SD_DMA_CACHE_MAINTENANCE  1
#define ENABLE_SCRATCH_BUFFER 1
 
#if defined(ENABLE_SCRATCH_BUFFER)
#if defined (ENABLE_SD_DMA_CACHE_MAINTENANCE)
ALIGN_32BYTES(static uint8_t scratch[BLOCKSIZE]); // 32-Byte aligned for cache maintenance
#else
__ALIGN_BEGIN static uint8_t scratch[BLOCKSIZE] __ALIGN_END;
#endif
#endif


SD_HandleTypeDef hsd1;
static volatile DSTATUS SD_State = STA_NOINIT;
static volatile UINT    ReadStatus  = 0;
static volatile UINT    WriteStatus = 0;

static void SD_ClockConfig(void)
{

}


uint8_t MX_SDMMC1_SD_Init(void)
{

  /* USER CODE BEGIN SDMMC1_Init 0 */

  /* USER CODE END SDMMC1_Init 0 */

  /* USER CODE BEGIN SDMMC1_Init 1 */

  /* USER CODE END SDMMC1_Init 1 */
  hsd1.Instance = SDMMC1;
  hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd1.Init.BusWide = SDMMC_BUS_WIDE_4B;
  hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd1.Init.ClockDiv = 5;

  if (HAL_SD_Init(&hsd1) != HAL_OK)
  {
    return  HAL_ERROR;
  }
  /* USER CODE BEGIN SDMMC1_Init 2 */

  /* USER CODE END SDMMC1_Init 2 */
    return HAL_OK;
}

void HAL_SD_MspInit(SD_HandleTypeDef* sdHandle)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(sdHandle->Instance==SDMMC1)
  {
  /* USER CODE BEGIN SDMMC1_MspInit 0 */

  /* USER CODE END SDMMC1_MspInit 0 */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SDMMC;
    PeriphClkInitStruct.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }
    /* SDMMC1 clock enable */
    __HAL_RCC_SDMMC1_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**SDMMC1 GPIO Configuration
    PC12     ------> SDMMC1_CK
    PC10     ------> SDMMC1_D2
    PC11     ------> SDMMC1_D3
    PD2     ------> SDMMC1_CMD
    PC9     ------> SDMMC1_D1
    PC8     ------> SDMMC1_D0
    */
    GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_9
                          |GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC1;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC1;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* USER CODE BEGIN SDMMC1_MspInit 1 */

  /* USER CODE END SDMMC1_MspInit 1 */
  }
}

void HAL_SD_MspDeInit(SD_HandleTypeDef* sdHandle)
{

  if(sdHandle->Instance==SDMMC1)
  {
  /* USER CODE BEGIN SDMMC1_MspDeInit 0 */

  /* USER CODE END SDMMC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SDMMC1_CLK_DISABLE();

    /**SDMMC1 GPIO Configuration
    PC12     ------> SDMMC1_CK
    PC10     ------> SDMMC1_D2
    PC11     ------> SDMMC1_D3
    PD2     ------> SDMMC1_CMD
    PC9     ------> SDMMC1_D1
    PC8     ------> SDMMC1_D0
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_12|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_9
                          |GPIO_PIN_8);

    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2);

  /* USER CODE BEGIN SDMMC1_MspDeInit 1 */

  /* USER CODE END SDMMC1_MspDeInit 1 */
  }
}

static int32_t BSP_SD_GetCardState(uint32_t Instance)
{
  return (int32_t)((HAL_SD_GetCardState(&hsd1) == HAL_SD_CARD_TRANSFER ) ? SD_TRANSFER_OK : SD_TRANSFER_BUSY);
}

static DSTATUS SD_CheckStatus(BYTE lun)
{
  SD_State = STA_NOINIT;

  if(BSP_SD_GetCardState(0) == BSP_ERROR_NONE)
  {
    SD_State &= ~STA_NOINIT;
  }
  return SD_State;
}

static int SD_CheckStatusWithTimeout(uint32_t timeout)
{
  uint32_t timer = HAL_GetTick();
  /* block until SDIO IP is ready again or a timeout occur */
  while(HAL_GetTick() - timer < timeout)
  {
    if (BSP_SD_GetCardState(0) == SD_TRANSFER_OK)
    {
      return 0;
    }
  }

  return -1;
}

int32_t BSP_SD_Erase(uint32_t Instance, uint32_t BlockIdx, uint32_t BlocksNbr)
{
  int32_t ret;

  if(Instance >= SD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(HAL_SD_Erase(&hsd1, BlockIdx, BlockIdx + BlocksNbr) != HAL_OK)
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }
  /* Return BSP status */
  return ret;
}

int32_t BSP_SD_ReadBlocks_DMA(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t BlocksNbr)
{
  int32_t ret;

  if(Instance >= SD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(HAL_SD_ReadBlocks_DMA(&hsd1, (uint8_t *)pData, BlockIdx, BlocksNbr) != HAL_OK)
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }
  /* Return BSP status */
  return ret;
}

int32_t BSP_SD_WriteBlocks_DMA(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t BlocksNbr)
{
  int32_t ret;

  if(Instance >= SD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(HAL_SD_WriteBlocks_DMA(&hsd1, (uint8_t *)pData, BlockIdx, BlocksNbr) != HAL_OK)
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }
  /* Return BSP status */
  return ret;
}

int32_t BSP_SD_GetCardInfo(uint32_t Instance, HAL_SD_CardInfoTypeDef *CardInfo)
{
  int32_t ret;

  if(Instance >= SD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(HAL_SD_GetCardInfo(&hsd1, CardInfo) != HAL_OK)
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }
  /* Return BSP status */
  return ret;
}

/**
  * @brief Rx Transfer completed callbacks
  * @param hsd: SD handle
  * @retval None
  */
void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd)
{
    ReadStatus = 1;
}

/**
  * @brief Tx Transfer completed callbacks
  * @param hsd: SD handle
  * @retval None
  */
void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd)
{
    WriteStatus = 1;
}

/******************************************************************************/
/* STM32H7xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32h7xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles SDMMC1 global interrupt.
  */
void SDMMC1_IRQHandler(void)
{
  HAL_SD_IRQHandler(&hsd1);
}

__weak uint8_t BSP_SD_ReadBlocks(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks, uint32_t Timeout)
{
  uint8_t sd_state = MSD_OK;

  if (HAL_SD_ReadBlocks(&hsd1, (uint8_t *)pData, ReadAddr, NumOfBlocks, Timeout) != HAL_OK)
  {
    sd_state = MSD_ERROR;
  }

  return sd_state;
}

__weak uint8_t BSP_SD_WriteBlocks(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks, uint32_t Timeout)
{
  uint8_t sd_state = MSD_OK;

  if (HAL_SD_WriteBlocks(&hsd1, (uint8_t *)pData, WriteAddr, NumOfBlocks, Timeout) != HAL_OK)
  {
    sd_state = MSD_ERROR;
  }

  return sd_state;
}

/* Register function */

DSTATUS SD_initialize(BYTE pdrv)
{

    //SD_ClockConfig(); 
    
    // if(MX_SDMMC1_SD_Init()== BSP_ERROR_NONE)
    // {
        SD_State = SD_CheckStatus(pdrv);
    //}
    return SD_State;
}

DSTATUS SD_status(BYTE pdrv)
{
    return SD_CheckStatus(pdrv);
}

DRESULT SD_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count)
{
    DRESULT res = RES_ERROR;

  if(BSP_SD_ReadBlocks((uint32_t*)buff,
                       (uint32_t) (sector),
                       count, SD_TIMEOUT) == MSD_OK)
  {
    /* wait until the read operation is finished */
    while(BSP_SD_GetCardState(0)!= MSD_OK)
    {
    }
    res = RES_OK;
  }
   return res;
}

DRESULT SD_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count)
{
    DRESULT res = RES_ERROR;

  if(BSP_SD_WriteBlocks((uint32_t*)buff,
                        (uint32_t)(sector),
                        count, SD_TIMEOUT) == 0)
  {
	/* wait until the Write operation is finished */
    while(BSP_SD_GetCardState(0) != 0)
    {
    }
    res = RES_OK;
  }

  return res;
}

DRESULT SD_ioctl(BYTE pdrv, BYTE cmd, void* buff)
{
  uint8_t res = RES_ERROR;
  HAL_SD_CardInfoTypeDef CardInfo;

  if (SD_State & STA_NOINIT) return RES_NOTRDY;

  switch (cmd)
  {
  /* Make sure that no pending write process */
  case CTRL_SYNC :
    res = RES_OK;
    break;

  /* Get number of sectors on the disk (DWORD) */
  case GET_SECTOR_COUNT :
    res = BSP_SD_GetCardInfo(0, &CardInfo);
    *(DWORD*)buff = CardInfo.LogBlockNbr;
    break;

  /* Get R/W sector size (WORD) */
  case GET_SECTOR_SIZE :
    res = BSP_SD_GetCardInfo(0, &CardInfo);
    *(WORD*)buff = CardInfo.LogBlockSize;
    break;

  /* Get erase block size in unit of sector (DWORD) */
  case GET_BLOCK_SIZE :
    res = BSP_SD_GetCardInfo(0, &CardInfo);
    *(DWORD*)buff = CardInfo.LogBlockSize / SD_DEFAULT_BLOCK_SIZE;
    break;

  default:
    res = RES_PARERR;
  }
  return (DRESULT)res;
}

#endif