#include "pwm.h"
/*
函数功能：定时器3-PWM波形输出配置

硬件PWM通道：PA6 PA7 PB0 PB1
功能说明：配置定时器3的通道1输出PWM波形
*/
//void Timer3_PWM_Init(u16 psc,u16 arr)
//{
//	  /*1. 开时钟*/
//	  RCC->APB1ENR|=1<<1;    //开启定时器3的时钟
//	  RCC->APB1RSTR|=1<<1;   //开启复位时钟  
//	  RCC->APB1RSTR&=~(1<<1);//关闭  
//	
//	  /*2. 配置定时器的核心寄存器*/
//	  TIM3->PSC=psc-1;     //预分频
//	  /*计数器的时钟频率CK_CNT等于fCK_PSC/(PSC[15:0]+1)*/
//	  TIM3->ARR=arr;       //重装载寄存器
//		
//	  /*3.  配置PWM波形相关寄存器*/
//		TIM3->CCMR1&=~(0x3<<0);
//		TIM3->CCMR1|=0x0<<0;  //CC1通道被配置为输出
//	  TIM3->CCMR1&=~(0x7<<4);
//		//TIM3->CCMR1|=0x6<<4; //模式1
//		TIM3->CCMR1|=0x7<<4;   //模式2
//		TIM3->CCER|=1<<0;      //OC1信号输出到对应的输出引脚
//		TIM3->CCR1=arr;      //占空比 50%
//		
//		/*4. 配置PWM波形输出的GPIO口*/
//	  RCC->AHB1ENR|=1<<0;			   //使能PORTA时钟
//		
//		GPIOA->MODER&=~(0x3<<6*2); //清除模式
//		GPIOA->MODER|=0x2<<6*2;    //配置复用功能模式
//		
//		GPIOA->OTYPER&=~(0x1<<6);  //0表示推挽输出
//		
//		GPIOA->OSPEEDR&=~(0x3<<6*2); //清除之前配置
//		GPIOA->OSPEEDR|=0x2<<6*2;    //50MHZ输出速度
//		
//		GPIOA->AFR[0]&=~(0xF<<4*6); //清除PA6配置
//		GPIOA->AFR[0]|=0x2<<4*6;    //配置PA6复用功能模式为定时器3的通道1
//		
//		/*5. 开启定时器*/
//		TIM3->CR1|=1<<0;
//}
void TIM3_PWM_Init(uint16_t arr, uint16_t psc)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    
    // 使能时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);    // TIM3时钟使能
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);   // GPIOA时钟使能
    
    // 配置PA6为复用功能
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;           // 复用功能
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;      // 高速
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;          // 推挽输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;            // 上拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 将PA6复用为TIM3_CH1
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_TIM3);
    
    // 初始化TIM3时基单元
    TIM_TimeBaseStructure.TIM_Period = arr;                 // 自动重装载值
    TIM_TimeBaseStructure.TIM_Prescaler = psc;              // 预分频器
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数模式
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; // 时钟分频因子
//    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;        // 重复计数器值(高级定时器用)
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    
    // 初始化TIM3通道1 PWM模式
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;       // PWM模式1
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // 输出使能
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; // 输出极性:高电平有效
    TIM_OCInitStructure.TIM_Pulse = 60;                      // 初始占空比为0
    TIM_OC1Init(TIM3, &TIM_OCInitStructure);                // 初始化通道1
    
    // 使能TIM3在CCR1上的预装载寄存器
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
    
    // 使能TIM3的自动重装载预装载寄存器
    TIM_ARRPreloadConfig(TIM3, ENABLE);
    
    // 启动TIM3计数器
    TIM_Cmd(TIM3, ENABLE);
}
