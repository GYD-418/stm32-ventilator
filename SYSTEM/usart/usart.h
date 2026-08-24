#ifndef __usart_H
#define __usart_H

#include "sys.h" 
#include "stm32f4xx.h"
#include "stdio.h"	

void USART3_Init(u32 bound);
void USART4_Init(u32 bound);
void Serial_SendByte(uint8_t Byte);
void Serial_SendArray(uint8_t *Array, uint16_t Length);
void Serial_SendString(char *String);
uint32_t Serial_Pow(uint32_t X, uint32_t Y);
void Serial_SendNumber(uint32_t Number, uint8_t Length);
void Serial_4_Printf(char *format, ...);
#endif


