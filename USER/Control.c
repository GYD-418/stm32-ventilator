#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "Control.h"



// 系统时钟配置
void RCC_Configuration(void) {
    // 启用外部高速时钟
    RCC_HSEConfig(RCC_HSE_ON);
    
    // 等待HSE稳定
    while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET);
    
    // 配置PLL
    RCC_PLLConfig(RCC_PLLSource_HSE, 8, 336, 2, 7);
    
    // 启用PLL
    RCC_PLLCmd(ENABLE);
    
    // 等待PLL稳定
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
    
    // 配置系统时钟源为PLL
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
    
    // 配置AHB时钟 (HCLK = SYSCLK)
    RCC_HCLKConfig(RCC_SYSCLK_Div1);
    
    // 配置APB1时钟 (PCLK1 = HCLK/4)
    RCC_PCLK1Config(RCC_HCLK_Div4);
    
    // 配置APB2时钟 (PCLK2 = HCLK/2)
    RCC_PCLK2Config(RCC_HCLK_Div2);
    
    // 使能GPIOD时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
}

// GPIO配置
void GPIO_Configuration(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 配置PD4和PD5为推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;          // 输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;         // 推挽输出
    GPIO_InitStructure.GPIO_PuPd =  GPIO_PuPd_NOPULL;       // 无上下拉
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;     // 输出速度100MHz
    
    // 应用配置到GPIOD
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    // 设置PD4和PD5为低电平
    GPIO_ResetBits(GPIOD, GPIO_Pin_8 | GPIO_Pin_9);
//    GPIO_SetBits(GPIOD, GPIO_Pin_8);
}

//#ifdef USE_FULL_ASSERT
//void assert_failed(uint8_t* file, uint32_t line) {
//    while (1) {
//    }
//}
//#endif
//    
