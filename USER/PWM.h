#ifndef __PWM_H
#define __PWM_H

#include "stm32f4xx.h"                  // Device header
#include "sys.h"
void Timer3_PWM_Init(u16 psc,u16 arr);
void TIM3_PWM_Init(uint16_t arr, uint16_t psc);
#endif
