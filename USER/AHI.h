#ifndef __AHI_H
#define __AHI_H

#include "stm32f4xx.h"
#include <string.h>
#include <stdio.h>

// ===================== AHI算法配置参数 =====================
#define AHI_RECORD_HOURS 8              // 记录时长（小时）
#define AHI_SAMPLE_INTERVAL_MS 1000     // 采样间隔（毫秒）
#define APNEA_THRESHOLD_PERCENT 90      // 呼吸暂停阈值（相对于正常流量的90%）
#define HYPOPNEA_THRESHOLD_PERCENT 50   // 低通气阈值（相对于正常流量的50%）
#define APNEA_MIN_DURATION_MS 10000     // 呼吸暂停最小时长（10秒）
#define HYPOPNEA_MIN_DURATION_MS 10000  // 低通气最小时长（10秒）
#define NORMAL_FLOW_BASELINE 1500       // 正常流量基线（mL/s，可根据实际情况调整）
#define FLOW_BUFFER_SIZE 60             // 流量缓冲区大小（60秒）
#define BASELINE_BUFFER_SIZE 10         // 基线缓冲区大小

// 事件类型定义
#define EVENT_TYPE_NORMAL     0  // 正常
#define EVENT_TYPE_APNEA      1  // 呼吸暂停
#define EVENT_TYPE_HYPOPNEA   2  // 低通气

// 严重程度等级
#define SEVERITY_NORMAL       0  // 正常 (AHI < 5)
#define SEVERITY_MILD         1  // 轻度 (5 ≤ AHI < 15)
#define SEVERITY_MODERATE     2  // 中度 (15 ≤ AHI < 30)
#define SEVERITY_SEVERE       3  // 重度 (AHI ≥ 30)

// 呼吸事件结构体
typedef struct {
    uint32_t event_start_time;       // 事件开始时间（毫秒）
    uint32_t event_duration;         // 事件持续时间（毫秒）
    uint8_t event_type;              // 0:正常 1:呼吸暂停 2:低通气
    uint16_t avg_flow;               // 事件期间平均流量
    uint16_t min_flow;               // 事件期间最小流量
    uint16_t max_flow;               // 事件期间最大流量
} SleepApneaEvent;

// AHI统计结构体
typedef struct {
    uint32_t record_start_tick;      // 记录开始时间
    uint32_t total_duration_ms;      // 总记录时长（毫秒）
    uint16_t apnea_count;            // 呼吸暂停次数
    uint16_t hypopnea_count;         // 低通气次数
    uint16_t total_events;           // 总事件数
    float ahi_value;                 // AHI值
    uint8_t severity_level;          // 0:正常 1:轻度 2:中度 3:重度
    uint8_t is_recording;            // 是否在记录中
    uint8_t current_event_active;    // 当前是否有活跃事件
    SleepApneaEvent current_event;   // 当前事件
    uint16_t flow_buffer[FLOW_BUFFER_SIZE];     // 最近60秒流量缓冲区
    uint8_t flow_buffer_idx;                    // 流量缓冲区索引
    uint32_t last_sample_time;                  // 上次采样时间
    uint16_t normal_flow_baseline;              // 动态基线流量
    uint16_t baseline_buffer[BASELINE_BUFFER_SIZE];  // 基线缓冲区
    uint8_t baseline_idx;                       // 基线缓冲区索引
    uint8_t baseline_filled;                    // 基线缓冲区是否已满
    uint32_t baseline_sum;                      // 基线总和
    uint32_t last_display_time;                 // 上次显示时间
    uint32_t last_sd_log_time;                  // 上次SD卡记录时间
} AHI_Statistics;

// 函数声明
void AHI_Init(void);
void AHI_Tick_Update(void);
void AHI_Update(uint16_t current_flow);
void AHI_Update_Baseline(uint16_t current_flow);
void AHI_Check_Event(uint16_t current_flow);
void AHI_Calculate_Index(void);
uint8_t AHI_Get_Severity_Level(float ahi);
const char* AHI_Get_Severity_String(uint8_t level);
void AHI_Start_Recording(void);
void AHI_Stop_Recording(void);
void AHI_Reset_Recording(void);
void AHI_Process_And_Display(void);
void SD_Log_AHI_Data(uint8_t sd_init_flag, void (*sd_write_func)(const char*));
float AHI_Get_Value(void);
uint16_t AHI_Get_Apnea_Count(void);
uint16_t AHI_Get_Hypopnea_Count(void);
uint8_t AHI_Is_Recording(void);
uint32_t AHI_Get_Record_Duration(void);
void AHI_Set_Baseline(uint16_t baseline);
void AHI_Export_Report(char* report_buffer, uint16_t buffer_size);

#endif /* __AHI_H */