#ifndef __IIC_H
#define __IIC_H
#include "stm32f4xx.h"                  // Device header
#include "sys.h"

void I2C1_Init(void);
void I2C1_Start(void);
void I2C1_Stop(void);
uint8_t I2C_WriteData(uint8_t addr, uint8_t cmd, uint8_t* data, uint8_t len);
uint8_t I2C_ReadData(uint8_t addr, uint8_t cmd, uint8_t* data, uint8_t len);
uint8_t OpenWriteProtect(void);
uint8_t SetSensorI2CAddr(uint8_t new_addr_7bit);
uint8_t QueryTemperature(uint16_t* temp);
//uint8_t QueryInstantFlow(int32_t* flow);
uint8_t QueryInstantFlow(int32_t* flow);
//float QueryInstantFlow(void);
uint8_t QuerySerialNumber(uint8_t* serial, uint8_t max_len) ;
uint8_t QueryHumidity(uint16_t* humidity);
uint8_t QueryPressure(int32_t* pressure);

#endif
