#include "IIC.h"
#include "string.h"

// -------------------------- 1. 宏定义 --------------------------
#define I2Cx                     I2C1  // 统一I2C外设别名
#define SENSOR_I2C_ADDR_7BIT     0x01  // 7位I2C地址
#define WRITE_PROTECT_CMD        0x9D  // 写保护命令码
#define WRITE_PROTECT_DATA       {0x53, 0x49, 0x41, 0x52}  // 写保护数据
#define SET_I2C_ADDR_CMD         0x05  // 设置I2C地址命令码
#define QUERY_TEMP_CMD           0xB2  // 查询温度命令码
#define QUERY_FLOW_CMD           0x84  // 查询瞬时流量命令码
#define QUERY_SERIAL_CMD         0x83  // 查询传感器序列号命令码
#define I2C_TIMEOUT_US           10000 // 超时时间
#define QUERY_HUMIDITY_COMMAND   0xB3  // 查询湿度命令码
#define QUERY_PRESSURE_CMD       0xA3  // 查询压力传感器命令码
// -------------------------- 2. I2C1初始化 --------------------------
void I2C1_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    I2C_InitTypeDef   I2C_InitStructure;

    // 使能GPIOB和I2C1时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    // 配置GPIO
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 配置GPIO复用映射
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1);

    // 初始化I2C1
    I2C_DeInit(I2Cx);
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0x00;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = 100000;
    I2C_Init(I2Cx, &I2C_InitStructure);

    // 使能I2C1
    I2C_Cmd(I2Cx, ENABLE);
}

// -------------------------- 3. 基础辅助函数 --------------------------
// I2C发送起始信号（带超时）
void I2C1_Start(void)
{
    uint32_t timeout = I2C_TIMEOUT_US;
    I2C_GenerateSTART(I2Cx, ENABLE);
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if (--timeout == 0) break;
    }
}

// I2C发送停止信号
void I2C1_Stop(void)
{
    I2C_GenerateSTOP(I2Cx, ENABLE);
    while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_STOPF))
    {
        I2C_ClearFlag(I2Cx, I2C_FLAG_STOPF);
    }
}

// -------------------------- 4. 通用I2C写函数 --------------------------
uint8_t I2C_WriteData(uint8_t addr, uint8_t cmd, uint8_t* data, uint8_t len) {
    uint32_t timeout = I2C_TIMEOUT_US;

    while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY))
    {
        if (--timeout == 0) return 0;
    }

    I2C_GenerateSTART(I2Cx, ENABLE);
    timeout = I2C_TIMEOUT_US;
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if (--timeout == 0) { I2C_GenerateSTOP(I2Cx, ENABLE); return 0; }
    }

    I2C_Send7bitAddress(I2Cx, addr << 1, I2C_Direction_Transmitter);
    timeout = I2C_TIMEOUT_US;
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        if (--timeout == 0) { I2C_GenerateSTOP(I2Cx, ENABLE); return 0; }
    }

    I2C_SendData(I2Cx, cmd);
    timeout = I2C_TIMEOUT_US;
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if (--timeout == 0) { I2C_GenerateSTOP(I2Cx, ENABLE); return 0; }
    }

    for (uint8_t i = 0; i < len; i++) {
        I2C_SendData(I2Cx, data[i]);
        timeout = I2C_TIMEOUT_US;
        while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
        {
            if (--timeout == 0) { I2C_GenerateSTOP(I2Cx, ENABLE); return 0; }
        }
    }

    I2C_GenerateSTOP(I2Cx, ENABLE);
    return 1;
}

// -------------------------- 5. 通用I2C读函数 --------------------------
uint8_t I2C_ReadData(uint8_t addr, uint8_t cmd, uint8_t* data, uint8_t len) {
    uint32_t timeout = I2C_TIMEOUT_US;

    while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY))
    {
        if (--timeout == 0) return 0;
    }

    I2C_GenerateSTART(I2Cx, ENABLE);
    timeout = I2C_TIMEOUT_US;
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if (--timeout == 0) { I2C_GenerateSTOP(I2Cx, ENABLE); return 0; }
    }

    I2C_Send7bitAddress(I2Cx, addr << 1, I2C_Direction_Transmitter);
    timeout = I2C_TIMEOUT_US;
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        if (--timeout == 0) { I2C_GenerateSTOP(I2Cx, ENABLE); return 0; }
    }

    I2C_SendData(I2Cx, cmd);
    timeout = I2C_TIMEOUT_US;
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if (--timeout == 0) { I2C_GenerateSTOP(I2Cx, ENABLE); return 0; }
    }

    I2C_GenerateSTART(I2Cx, ENABLE);
    timeout = I2C_TIMEOUT_US;
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if (--timeout == 0) { I2C_GenerateSTOP(I2Cx, ENABLE); return 0; }
    }

    I2C_Send7bitAddress(I2Cx, addr << 1, I2C_Direction_Receiver);
    timeout = I2C_TIMEOUT_US;
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
    {
        if (--timeout == 0) { I2C_GenerateSTOP(I2Cx, ENABLE); return 0; }
    }

    for (uint8_t i = 0; i < len; i++) {
        if (i == len - 1) {
            I2C_AcknowledgeConfig(I2Cx, DISABLE);
        }

        timeout = I2C_TIMEOUT_US;
        while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED))
        {
            if (--timeout == 0) {
                I2C_AcknowledgeConfig(I2Cx, ENABLE);
                I2C_GenerateSTOP(I2Cx, ENABLE);
                return 0;
            }
        }
        data[i] = I2C_ReceiveData(I2Cx);
    }

    I2C_GenerateSTOP(I2Cx, ENABLE);
    I2C_AcknowledgeConfig(I2Cx, ENABLE);
    return 1;
}

// -------------------------- 6. 传感器业务函数 --------------------------
uint8_t OpenWriteProtect(void) {
    uint8_t data[4] = WRITE_PROTECT_DATA;
    return I2C_WriteData(SENSOR_I2C_ADDR_7BIT, WRITE_PROTECT_CMD, data, 4);
}

uint8_t SetSensorI2CAddr(uint8_t new_addr_7bit) {
    if (new_addr_7bit > 0x7F) return 0;

    if (!OpenWriteProtect()) {
        return 0;
    }
    return I2C_WriteData(SENSOR_I2C_ADDR_7BIT, SET_I2C_ADDR_CMD, &new_addr_7bit, 1);
}

uint8_t QueryTemperature(uint16_t* temp) {
    uint8_t data[3] = {0};

    if (!I2C_ReadData(SENSOR_I2C_ADDR_7BIT, QUERY_TEMP_CMD, data, 3)) {
        return 0;
    }

    *temp = (data[0] << 8) | data[1];

    uint8_t crc_calc = data[0] ^ data[1];
    if (crc_calc != data[2]) {
        return 0;
    }

    return 1;
}

//uint8_t QueryInstantFlow(int32_t* flow) {
//    uint8_t data[3] = {0};

//    if (!I2C_ReadData(SENSOR_I2C_ADDR_7BIT, QUERY_FLOW_CMD, data, 3)) {
//        return 0;
//    }

//    *flow = (int16_t)((data[0] << 8) | data[1]);

//    uint8_t crc_calc = data[0] ^ data[1];
//    if (crc_calc != data[2]) {
//        return 0;
//    }

//    return 1;
//}
uint8_t QueryInstantFlow(int32_t* flow) {
uint8_t data[5] = {0};  // 4字节流量数据 + 1字节CRC// 参数合法性检查
if (flow == NULL) {
return 0;
}// 发送查询流量命令并读取5字节数据
if (!I2C_ReadData(SENSOR_I2C_ADDR_7BIT, 0x83, data, 5)) {
return 0;  // 读取失败（超时）
}// 组合32位流量原始数据
*flow = (int32_t)((data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3]);// CRC校验（前4字节异或）
uint8_t crc_calc = data[0] ^ data[1] ^ data[2] ^ data[3];
if (crc_calc != data[4]) {
return 0;  // CRC不匹配，数据错误
}return 1;  // 成功
}
uint8_t QuerySerialNumber(uint8_t* serial, uint8_t max_len) {
    if (serial == NULL || max_len < 12) {
        return 0;
    }

    if (!I2C_ReadData(SENSOR_I2C_ADDR_7BIT, QUERY_SERIAL_CMD, serial, 12)) {
        return 0;
    }

    return 1;
}

// -------------------------- 新增：查询湿度函数 --------------------------
/**
 * @brief   查询传感器的湿度数据
 * @param   humidity: 指向存储湿度数据的指针
 * @retval  1: 成功，0: 失败（超时或CRC校验错误）
 * @note    湿度数据格式：16位无符号整数，具体解析需参考传感器手册
 */
uint8_t QueryHumidity(uint16_t* humidity) {
    uint8_t data[3] = {0};  // 存储读取到的数据：湿度高8位+低8位+CRC校验位
    
    // 参数合法性检查
    if (humidity == NULL) {
        return 0;
    }
    
    // 发送查询湿度命令并读取3字节数据
    if (!I2C_ReadData(SENSOR_I2C_ADDR_7BIT, QUERY_HUMIDITY_COMMAND, data, 3)) {
        return 0;  // 读取失败（超时）
    }
    
    // 组合湿度数据（高8位左移8位 + 低8位）
    *humidity = (data[0] << 8) | data[1];
    
    // CRC校验（与温度查询使用相同的校验方式）
    uint8_t crc_calc = data[0] ^ data[1];  // 计算校验值
    if (crc_calc != data[2]) {
        return 0;  // CRC不匹配，数据错误
    }
    
    return 1;  // 成功
}
/**
 * @brief   查询传感器的压力数据
 * @param   pressure: 指向存储压力数据的指针
 * @retval  1: 成功，0: 失败（超时或CRC校验错误）
 * @note    压力数据格式：16位无符号整数，具体解析需参考传感器手册
 */
uint8_t QueryPressure(int32_t* pressure) {
     uint8_t data[5] = {0};  // 4字节压力数据 + 1字节CRC
     
     // 参数合法性检查
     if (pressure == NULL) {
         return 0;
     }
     
     // 发送查询压力命令并读取5字节数据
     if (!I2C_ReadData(SENSOR_I2C_ADDR_7BIT, QUERY_PRESSURE_CMD, data, 5)) {
         return 0;  // 读取失败（超时）
     }
     
     // 组合32位压力原始数据
     *pressure = (int32_t)((data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3]);
     
     // CRC校验（前4字节异或）
     uint8_t crc_calc = data[0] ^ data[1] ^ data[2] ^ data[3];
     if (crc_calc != data[4]) {
         return 0;  // CRC不匹配，数据错误
     }
     
     return 1;  // 成功
 }


