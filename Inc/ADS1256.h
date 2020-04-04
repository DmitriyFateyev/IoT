#ifndef MAX31865_H
#define MAX31865_H

#include "stm32f1xx_hal.h"






/*---------------------------------------------------------------------------*
  Variables
 *---------------------------------------------------------------------------*/
extern volatile uint32_t ADS1256_DRDY_FLAG;
extern volatile uint32_t ADS1256_CNVRT_COUNTER;
extern float ADS1256_VoltCONVERT;




/*---------------------------------------------------------------------------*
  !!! Configure ChipSelect ports&pins before use !!!
 *---------------------------------------------------------------------------*/
/* Defines *******************************************************************/
#define		ADS1256_SPI					hspi2			/* MUST BEE DEFINED! */

#define		ADS1256_CS_GPIO			GPIOB
#define		ADS1256_CS_GPIO_Pin		GPIO_PIN_12

#define 		ADS1256_RESET_GPIO_Port GPIOA
#define 		ADS1256_RESET_Pin 		GPIO_PIN_9

#define		ADS1256_CS_HIGH()			HAL_GPIO_WritePin(ADS1256_CS_GPIO, ADS1256_CS_GPIO_Pin, GPIO_PIN_SET);
#define		ADS1256_CS_LOW()			HAL_GPIO_WritePin(ADS1256_CS_GPIO, ADS1256_CS_GPIO_Pin, GPIO_PIN_RESET);

#define		ADS1256_START_RESET()	HAL_GPIO_WritePin(ADS1256_RESET_GPIO_Port, ADS1256_RESET_Pin, GPIO_PIN_RESET);
#define		ADS1256_STOP_RESET()		HAL_GPIO_WritePin(ADS1256_RESET_GPIO_Port, ADS1256_RESET_Pin, GPIO_PIN_SET);

#define 		CHECK_BIT(var,pos) ((var) & (1<<(pos)))


void ADS1256_Init(void);
uint32_t ADS1256_ReadData (void);
void ADS1256_SELFCAL(void);
void ADS1256_read_regs(void);
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max);


#endif
