#ifndef	__HMI_H__
#define __HMI_H__
 
#include "sys.h"
#include "stm32f4xx.h"                  // Device header
void printf_usart1(const char* format, ...);
void HMI_send_string(char* name, char* showdata);
void HMI_send_number(char* name, int num);
void HMI_send_float(char* name, float num);
void HMI_Wave(char* name, int ch, int val);
void HMI_Wave_Fast(char* name, int ch, int count, int* show_data);
void HMI_Wave_Clear(char* name, int ch);
#endif
