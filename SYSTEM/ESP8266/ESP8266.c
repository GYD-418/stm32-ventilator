//#include "usart.h"
//#include "delay.h"
//#include "ESP8266.h"
////定义每个需要的值
////#define WIFIID huawei
////#define WIFIMIMA guoyaodong4518...
////#define DIZHI 192.168.43.1
////#define DUANKOU 9999
////#define USERNAME 66534f1e6bc31504f06c08d5_4518
////#define PASSWORD f91d73ef3ef34446a7c302cef4638ed82b9a0c1e3c7ef1df18494f773119de09

//int Temperature=10;
//int  Pressure,Humidity=10;
//int	 FlowRate=10;
//int	 RespiratoryRatio = 10;

//void ESP8266_Init(void)
//{
//	Serial_4_Printf("AT\r\n");
//	delay_ms(20);
//	Serial_4_Printf("AT+CWMODE=1\r\n");
//	delay_ms(1);
//	Serial_4_Printf("AT+RST\r\n");	
//	delay_ms(1);
//	Serial_4_Printf("AT\r\n");
//	delay_ms(20);
//	Serial_4_Printf("AT+CWJAP=\"iQOO Neo9S Pro\",\"guoyaodong4518...\"\r\n");
//	delay_ms(3);
//	Serial_4_Printf("AT+CIPMUX=0\r\n");
//	delay_ms(20);
//	Serial_4_Printf("AT+CIPMODE=1\r\n");
//	delay_ms(20);
//	Serial_4_Printf("AT+CIPSTART=\"TCP\",\"192.168.43.1\",9999\r\n");
//	delay_ms(1);
//	Serial_4_Printf("AT+CIPSEND\r\n");
//}

//void ESP8266_Huaweiyun_Init(void)			//上云
//{
//	Serial_4_Printf("AT+CWMODE=1\r\n");
//	delay_ms(2000);
//	Serial_4_Printf("AT+RST\r\n");	
//	delay_ms(1000);
////	Serial_4_Printf("AT+CWJAP=\"iQOO-Neo9S-Pro\",\"guoyaodong4518...\"\r\n");
//	Serial_4_Printf("AT+CWJAP=\"OnePlus Ace 2\",\"jingyi1002\"\r\n");
//	delay_ms(2000);
//	Serial_4_Printf("AT+MQTTUSERCFG=0,1,\"NULL\",\"667f82db6bc31504f0711afb_ximenzi\",\"651218546c008a3076a546828aaf77466bbc60ccf8008a81ab7f2a60ed4313aa\",0,0,\"\"\r\n");
//	delay_ms(2000);
//	Serial_4_Printf("AT+MQTTCLIENTID=0,\"667f82db6bc31504f0711afb_ximenzi_0_0_2025101310\"\r\n");
//	delay_ms(2000);
//	Serial_4_Printf("AT+MQTTCONN=0,\"33af8f4e16.st1.iotda-device.cn-north-4.myhuaweicloud.com\",1883,1\r\n");
//	delay_ms(2000);
//	Serial_4_Printf("AT+MQTTSUB=0,\"$oc/devices/ximenzi/sys/properties/report\",1\r\n");
//	delay_ms(2000);
//	Serial_4_Printf("AT+MQTTPUB=0,\"$oc/devices/ximenzi/sys/properties/report\",\"{\\\"services\\\":[{\\\"service_id\\\":\\\"information\\\"\\,\\\"properties\\\":{\\\"Temperature\\\":%d\\,\\\"Pressure\\\":%d\\,\\\"Humidity\\\":%d\\,\\\"FlowRate\\\":%d\\,\\\"RespiratoryRatio\\\":%d\\,\\\"CumulativeFlow \\\":%d\\}}]}\",1,0\r\n",Temperature,Pressure,Humidity,FlowRate,RespiratoryRatio);	
//}
//void ESP8266_Huaweiyun_Init2(void)			//上云
//{
//	delay_ms(2000);
//	Serial_4_Printf("AT+MQTTCLIENTID=0,\"667f82db6bc31504f0711afb_ximenzi_0_0_2025100706\"\r\n");
//	delay_ms(2000);
//	Serial_4_Printf("AT+MQTTCONN=0,\"33af8f4e16.st1.iotda-device.cn-north-4.myhuaweicloud.com\",1883,1\r\n");
//	delay_ms(2000);
//	Serial_4_Printf("AT+MQTTSUB=0,\"$oc/devices/ximenzi/sys/properties/report\",1\r\n");

//}
#include "usart.h"
#include "delay.h"
#include "ESP8266.h"
#include "IIC.h"  // 包含IIC头文件

// 传感器数据变量定义
//uint16_t Temperature = 0;    // 温度（16位）
//uint16_t Humidity = 0;       // 湿度（16位）
//int32_t Pressure = 0;        // 压力（32位）
//int32_t FlowRate = 0;        // 流量（32位）
//int RespiratoryRatio = 10;   // 呼吸比（示例值）
//int CumulativeFlow = 0;      // 累计流量（示例值）
//uint8_t isFirstRead = 1;     // 首次读取标志位（控制流量和压力仅读一次）

// 初始化传感器和ESP8266
//void System_Init(void) {
//    I2C1_Init();          // 初始化IIC总线
//    delay_ms(100);        // 等待传感器初始化稳定
//}

//// 读取传感器数据（流量和压力仅首次读取）
//uint8_t ReadSensors(void) {
//    uint8_t ret = 1;
//    
//    // 每次都读取温度和湿度
//    if (!QueryTemperature(&Temperature)) {
//        ret = 0;
//    }
//    if (!QueryHumidity(&Humidity)) {
//        ret = 0;
//    }
//    
//    // 仅首次读取流量和压力
//    if (isFirstRead) {
//        if (!QueryPressure(&Pressure)) {
//            ret = 0;
//        }
//        if (!QueryInstantFlow(&FlowRate)) {
//            ret = 0;
//        }
//        isFirstRead = 0;  // 读取一次后置0，不再重复读取
//    }
//    
//    return ret;
//}

// 华为云MQTT连接并发送传感器数据
void ESP8266_Huaweiyun_Init(void) {			// 上云函数
    // 连接WiFi
    Serial_4_Printf("AT+CWMODE=1\r\n");
    delay_ms(2000);
    Serial_4_Printf("AT+RST\r\n");	
    delay_ms(1000);
    Serial_4_Printf("AT+CWJAP=\"OnePlus Ace 2\",\"jingyi1002\"\r\n");
    delay_ms(5000);  // 延长WiFi连接等待时间
    
    // MQTT配置
    Serial_4_Printf("AT+MQTTUSERCFG=0,1,\"NULL\",\"EDA_hxj_ahr\",\"7fcc96f69e92a2d00210940960dedbd144b7f382f702b36489a65f8eddbb3d42\",0,0,\"\"\r\n");
    delay_ms(2000);
    Serial_4_Printf("AT+MQTTCLIENTID=0,\"EDA_hxj_ahr_0_0_2026040703\"\r\n");
    delay_ms(2000);
    
    // 连接MQTT服务器
    Serial_4_Printf("AT+MQTTCONN=0,\"5498f7ad00.st1.iotda-device.cn-north-4.myhuaweicloud.com\",1883,1\r\n");
    delay_ms(3000);
    
    // 订阅主题（可选）
    Serial_4_Printf("AT+MQTTSUB=0,\"$oc/devices/EDA_hxj_ahr/sys/properties/report\",1\r\n");
    delay_ms(2000);
    
    // 读取传感器数据（流量和压力仅首次读取）
//    ReadSensors();
    
    // 发送传感器数据到华为云
//    Serial_4_Printf("AT+MQTTPUB=0,\"$oc/devices/ximenzi/sys/properties/report\",\"{\\\"services\\\":[{\\\"service_id\\\":\\\"information\\\"\\,\\\"properties\\\":{\\\"Temperature\\\":%d\\,\\\"Pressure\\\":%d\\,\\\"Humidity\\\":%d\\,\\\"FlowRate\\\":%d\\,\\\"RespiratoryRatio\\\":%d\\,\\\"CumulativeFlow\\\":%d}}]}\",1,0\r\n",
//                   Temperature, Pressure, Humidity, FlowRate, RespiratoryRatio, CumulativeFlow);	
//    delay_ms(2000);
}