#include "HIMI.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "usart.h"
#include "stm32f4xx.h"                  // Device header

// 实现通过USART1发送格式化字符串的函数
void printf_usart1(const char* format, ...)
{
    va_list args;
    char buffer[256];  // 缓冲区，足够存储大多数HMI指令
    
    // 格式化输出到缓冲区
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    // 通过USART1发送数据
    for (uint16_t i = 0; i < strlen(buffer); i++)
    {
        // 等待发送寄存器为空
        while (!(USART1->SR & USART_SR_TXE));
        // 发送一个字节
        USART1->DR = (buffer[i] & 0xFF);
    }
}

void HMI_send_string(char* name, char* showdata)
{
    printf_usart1("%s=\"%s\"\xff\xff\xff", name, showdata);
}
 
void HMI_send_number(char* name, int num)
{
    printf_usart1("%s=%d\xff\xff\xff", name, num);
}

void HMI_send_float(char* name, float num)
{
    // 乘以100保留两位小数，陶晶驰屏可以自动处理
    printf_usart1("%s=%.2f\xff\xff\xff", name, num);
}
 
void HMI_Wave(char* name, int ch, int val)
{
    printf_usart1("add %s,%d,%d\xff\xff\xff", name, ch, val);
}
 
void HMI_Wave_Fast(char* name, int ch, int count, int* show_data)
{
    int i;
    printf_usart1("addt %s,%d,%d\xff\xff\xff", name, ch, count);
    // 延时可以根据实际情况调整，不必太长
    for (i = 0; i < count; i++)
    {
        // 确保数据在有效范围内（0-255）
        uint8_t data = (show_data[i] < 0) ? 0 : (show_data[i] > 255 ? 255 : show_data[i]);
        while (!(USART1->SR & USART_SR_TXE));
        USART1->DR = data;
    }
    printf_usart1("\xff\xff\xff");
}
 
void HMI_Wave_Clear(char* name, int ch)
{
    printf_usart1("cle %s,%d\xff\xff\xff", name, ch);
}
