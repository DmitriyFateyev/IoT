/*----------------------------------------------------------------------------
 * Name:    ADS1256.c
 * Purpose: The ADS1256 are extremely low-noise,
				24-bit analog-to-digital (A/D) converter.
 *          
 * Version: V1.00
 * Note(s):
 *----------------------------------------------------------------------------
 * Copyright (c) 2016 NeoSteel Engineering. All rights reserved.
 *----------------------------------------------------------------------------
 * History: V1.00 Initial Version
 *----------------------------------------------------------------------------*/

#include "ADS1256.h"
#include "spi.h"

unsigned long adc_val =0; // store reading
int32_t read = 0;
volatile uint8_t first_read = SET;
float tmp=0;
uint8_t ADS1256_buffer[8];

volatile uint32_t ADS1256_DRDY_FLAG;
volatile uint32_t ADS1256_CNVRT_COUNTER;
float ADS1256_VoltCONVERT = (5.0f / 16777215.0f);

uint32_t bit32;
uint32_t bit24;


/*----------------------------------------------------------------------------
  Read ADS1256 SPI
 *----------------------------------------------------------------------------*/
uint8_t ADS1256_ReadSPI(uint8_t REG_ADDR) 
{
	hspi2.Instance->DR = REG_ADDR;
	while (!(__HAL_SPI_GET_FLAG(&hspi2, SPI_FLAG_RXNE)))
	{
		__nop();
	}
	REG_ADDR = hspi2.Instance->DR;	
	
	return REG_ADDR;
}


/*----------------------------------------------------------------------------
  Send byte to ADS1256 SPI
 *----------------------------------------------------------------------------*/
uint8_t ADS1256_sendByte(uint8_t byte) 
{
	while (!(__HAL_SPI_GET_FLAG(&hspi2, SPI_FLAG_TXE)))
	{
		__nop();
	}
	hspi2.Instance->DR = byte;
	while (!(__HAL_SPI_GET_FLAG(&hspi2, SPI_FLAG_RXNE)))
	{
		__nop();
	}
	byte = hspi2.Instance->DR;	
	
	return byte;
}


void ADS1256_RESET(void)
{
	ADS1256_START_RESET()
	HAL_Delay(10);
	ADS1256_STOP_RESET()
	HAL_Delay(10);
	ADS1256_DRDY_FLAG = RESET;
}


void ADS1256_Init(void)
{
	HAL_Delay(100);
	while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8))
	{
	}	
	ADS1256_RESET();	
	
	while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8))
	{
	}	
	ADS1256_CS_LOW()
	ADS1256_ReadSPI(0xFC); //SYNC
	ADS1256_ReadSPI(0xFF); //WAKE-UP
	ADS1256_CS_HIGH()
	while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8))
	{
	}	
	ADS1256_buffer[0] = 0x50; //WREG Command 
	ADS1256_buffer[1] = 0x04; //4byte (registers modified)
	
	ADS1256_buffer[2] = 0x06; //04 STATUS Reg = Auto-Calib Enabled, BUFFER OFF (DISABLE BUFFER FOR MEASURE 5V!)
	ADS1256_buffer[3] = 0x01; //MUX Reg = Positive:AIN0, Negative:AINCOM
	ADS1256_buffer[4] = 0x21; //ADCON Reg = GAIN = 1. INPUT: 0-5V
	ADS1256_buffer[5] = 0x23; //DRATE Reg = 0x03 = 2.5SPS; 0x23 = 10SPS; 0x63 = 50SPS
	ADS1256_buffer[6] = 0x00; //IO
	
	ADS1256_CS_LOW()
	HAL_SPI_Transmit(&hspi2, (uint8_t *)ADS1256_buffer, 7, 100);
	ADS1256_CS_HIGH()
	
	ADS1256_CNVRT_COUNTER=100;
	__HAL_SPI_ENABLE(&hspi2);
	ADS1256_DRDY_FLAG = RESET;
}


uint32_t ADS1256_ReadData (void)
{
	ADS1256_CS_LOW()
	ADS1256_ReadSPI(0x01);	//RDATA	
	for(volatile uint32_t i=0; i<70; i++)	//Pause for data prepare
	{
		__nop();
	}	
	
	ADS1256_buffer[0] = ADS1256_ReadSPI(0x00); //MSB
	ADS1256_buffer[1] = ADS1256_ReadSPI(0x00); //Mid-Byte
	ADS1256_buffer[2] = ADS1256_ReadSPI(0x00); //LSB
	ADS1256_CS_HIGH()	

	// construct 24 bit value
	read  = ((uint32_t)ADS1256_buffer[0] << 16) & 0x00FF0000;
	read |= ((uint32_t)ADS1256_buffer[1] << 8);
	read |= ADS1256_buffer[2];
	
	if (read & 0x800000) //Sign
	{
		read |= 0xFF000000;
	}

	return (uint32_t)(read+(uint32_t)0x800000); // return unsigned value 0 to 16777215 (0-5V)
}


/*----------------------------------------------------------------------------
  Self Calibration ADS1256
 *----------------------------------------------------------------------------*/
void ADS1256_SELFCAL(void) 
{
	while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8))
	{
	}		
	ADS1256_CS_LOW()
	ADS1256_sendByte(0xF0);
	ADS1256_CS_HIGH()	
	while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8))
	{
	}
	ADS1256_DRDY_FLAG = RESET;
}


/*----------------------------------------------------------------------------
  Read and print ADS1256 registers value
 *----------------------------------------------------------------------------*/
void ADS1256_read_regs(void) {
	while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8))
	{
	}
	ADS1256_CS_LOW()
	ADS1256_ReadSPI(0x10); //Read from Registers command
	ADS1256_ReadSPI(0x04); //Num of registers to read - 1 (4 - 0x03)
	
	for(volatile uint32_t i=0; i<20; i++)	{
		__nop();
	}	
	
	ADS1256_buffer[0] = ADS1256_ReadSPI(0x00);
	ADS1256_buffer[1] = ADS1256_ReadSPI(0x00);
	ADS1256_buffer[2] = ADS1256_ReadSPI(0x00);
	ADS1256_buffer[3] = ADS1256_ReadSPI(0x00);
	ADS1256_buffer[4] = ADS1256_ReadSPI(0x00);
	
	ADS1256_CS_HIGH()
	
	printf("\nREGISTERS:\n");
	printf("STATUS=0x%02X\n", ADS1256_buffer[0]);
	printf("MUX=    0x%02X\n",ADS1256_buffer[1]);
	printf("ADCON = 0x%02X\n",  ADS1256_buffer[2]);
	printf("DRATE = 0x%02X\n",  ADS1256_buffer[3]);
	printf("IO = 0x%02X\n",  ADS1256_buffer[4]);
}


float mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
