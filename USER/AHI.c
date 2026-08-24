#include "AHI.h"

// 全局AHI统计变量
static AHI_Statistics ahi_stats;
static uint32_t system_tick_ms = 0;

// 初始化AHI模块
void AHI_Init(void) {
    memset(&ahi_stats, 0, sizeof(AHI_Statistics));
    memset(ahi_stats.flow_buffer, 0, sizeof(ahi_stats.flow_buffer));
    memset(ahi_stats.baseline_buffer, 0, sizeof(ahi_stats.baseline_buffer));
    
    ahi_stats.flow_buffer_idx = 0;
    ahi_stats.normal_flow_baseline = NORMAL_FLOW_BASELINE;
    ahi_stats.is_recording = 0;
    ahi_stats.current_event_active = 0;
    ahi_stats.last_sample_time = 0;
    ahi_stats.record_start_tick = 0;
    ahi_stats.total_duration_ms = 0;
    ahi_stats.apnea_count = 0;
    ahi_stats.hypopnea_count = 0;
    ahi_stats.total_events = 0;
    ahi_stats.ahi_value = 0;
    ahi_stats.severity_level = 0;
    ahi_stats.baseline_idx = 0;
    ahi_stats.baseline_filled = 0;
    ahi_stats.baseline_sum = 0;
    ahi_stats.last_display_time = 0;
    ahi_stats.last_sd_log_time = 0;
}

// 系统时间更新（需要在1ms定时器中调用）
void AHI_Tick_Update(void) {
    system_tick_ms++;
}

// 更新动态基线流量（使用移动平均）
void AHI_Update_Baseline(uint16_t current_flow) {
    // 只使用正常呼吸阶段的流量更新基线（排除明显异常值）
    if (current_flow > 200 && current_flow < 3000) {
        if (ahi_stats.baseline_filled) {
            ahi_stats.baseline_sum -= ahi_stats.baseline_buffer[ahi_stats.baseline_idx];
        }
        ahi_stats.baseline_buffer[ahi_stats.baseline_idx] = current_flow;
        ahi_stats.baseline_sum += current_flow;
        ahi_stats.baseline_idx = (ahi_stats.baseline_idx + 1) % BASELINE_BUFFER_SIZE;
        
        if (ahi_stats.baseline_idx == 0) {
            ahi_stats.baseline_filled = 1;
        }
        
        if (ahi_stats.baseline_filled) {
            ahi_stats.normal_flow_baseline = ahi_stats.baseline_sum / BASELINE_BUFFER_SIZE;
            // 确保基线不低于最小值
            if (ahi_stats.normal_flow_baseline < 500) {
                ahi_stats.normal_flow_baseline = 500;
            }
        }
    }
}

// 检测呼吸事件
void AHI_Check_Event(uint16_t current_flow) {
    if (!ahi_stats.is_recording) return;
    
    uint16_t flow_percent;
    uint32_t current_time = system_tick_ms;
    
    // 避免除零错误
    if (ahi_stats.normal_flow_baseline == 0) {
        return;
    }
    
    flow_percent = (current_flow * 100) / ahi_stats.normal_flow_baseline;
    
    // 检测是否发生呼吸暂停或低通气
    if (flow_percent < APNEA_THRESHOLD_PERCENT) {
        // 呼吸暂停
        if (!ahi_stats.current_event_active) {
            // 开始新事件
            ahi_stats.current_event_active = 1;
            ahi_stats.current_event.event_start_time = current_time;
            ahi_stats.current_event.event_type = EVENT_TYPE_APNEA;
            ahi_stats.current_event.avg_flow = current_flow;
            ahi_stats.current_event.min_flow = current_flow;
            ahi_stats.current_event.max_flow = current_flow;
            ahi_stats.current_event.event_duration = 0;
        } else {
            // 更新当前事件
            ahi_stats.current_event.event_duration = current_time - ahi_stats.current_event.event_start_time;
            // 更新平均流量
            ahi_stats.current_event.avg_flow = (ahi_stats.current_event.avg_flow + current_flow) / 2;
            // 更新最小/最大流量
            if (current_flow < ahi_stats.current_event.min_flow) {
                ahi_stats.current_event.min_flow = current_flow;
            }
            if (current_flow > ahi_stats.current_event.max_flow) {
                ahi_stats.current_event.max_flow = current_flow;
            }
        }
    } 
    else if (flow_percent < HYPOPNEA_THRESHOLD_PERCENT) {
        // 低通气
        if (!ahi_stats.current_event_active) {
            ahi_stats.current_event_active = 1;
            ahi_stats.current_event.event_start_time = current_time;
            ahi_stats.current_event.event_type = EVENT_TYPE_HYPOPNEA;
            ahi_stats.current_event.avg_flow = current_flow;
            ahi_stats.current_event.min_flow = current_flow;
            ahi_stats.current_event.max_flow = current_flow;
            ahi_stats.current_event.event_duration = 0;
        } else if (ahi_stats.current_event.event_type == EVENT_TYPE_APNEA) {
            // 如果之前标记为呼吸暂停，但流量回升到低通气水平，重新分类
            ahi_stats.current_event.event_type = EVENT_TYPE_HYPOPNEA;
        }
        
        if (ahi_stats.current_event_active) {
            ahi_stats.current_event.event_duration = current_time - ahi_stats.current_event.event_start_time;
            ahi_stats.current_event.avg_flow = (ahi_stats.current_event.avg_flow + current_flow) / 2;
            if (current_flow < ahi_stats.current_event.min_flow) {
                ahi_stats.current_event.min_flow = current_flow;
            }
            if (current_flow > ahi_stats.current_event.max_flow) {
                ahi_stats.current_event.max_flow = current_flow;
            }
        }
    }
    else {
        // 正常呼吸，检查是否结束事件
        if (ahi_stats.current_event_active) {
            // 检查事件持续时间是否达到阈值
            uint32_t duration = ahi_stats.current_event.event_duration;
            
            if (ahi_stats.current_event.event_type == EVENT_TYPE_APNEA && duration >= APNEA_MIN_DURATION_MS) {
                // 有效呼吸暂停事件
                ahi_stats.apnea_count++;
                ahi_stats.total_events++;
                
                // 输出调试信息
                printf("AHI: Apnea detected - Duration: %d ms, Avg Flow: %d, Min Flow: %d\r\n", 
                       duration, ahi_stats.current_event.avg_flow, ahi_stats.current_event.min_flow);
            }
            else if (ahi_stats.current_event.event_type == EVENT_TYPE_HYPOPNEA && duration >= HYPOPNEA_MIN_DURATION_MS) {
                // 有效低通气事件
                ahi_stats.hypopnea_count++;
                ahi_stats.total_events++;
                
                printf("AHI: Hypopnea detected - Duration: %d ms, Avg Flow: %d, Min Flow: %d\r\n", 
                       duration, ahi_stats.current_event.avg_flow, ahi_stats.current_event.min_flow);
            }
            
            // 重置事件标志
            ahi_stats.current_event_active = 0;
            ahi_stats.current_event.event_duration = 0;
        }
    }
}

// 计算AHI指数
void AHI_Calculate_Index(void) {
    if (ahi_stats.total_duration_ms == 0) return;
    
    float hours = ahi_stats.total_duration_ms / 3600000.0f;
    if (hours > 0) {
        ahi_stats.ahi_value = ahi_stats.total_events / hours;
        ahi_stats.severity_level = AHI_Get_Severity_Level(ahi_stats.ahi_value);
    }
}

// 获取严重程度等级
uint8_t AHI_Get_Severity_Level(float ahi) {
    if (ahi < 5) return SEVERITY_NORMAL;
    else if (ahi < 15) return SEVERITY_MILD;
    else if (ahi < 30) return SEVERITY_MODERATE;
    else return SEVERITY_SEVERE;
}

// 获取严重程度描述字符串
const char* AHI_Get_Severity_String(uint8_t level) {
    switch(level) {
        case SEVERITY_NORMAL:   return "Normal";
        case SEVERITY_MILD:     return "Mild";
        case SEVERITY_MODERATE: return "Moderate";
        case SEVERITY_SEVERE:   return "Severe";
        default:                return "Unknown";
    }
}

// 开始记录AHI
void AHI_Start_Recording(void) {
    if (ahi_stats.is_recording) return;
    
    ahi_stats.is_recording = 1;
    ahi_stats.record_start_tick = system_tick_ms;
    ahi_stats.total_duration_ms = 0;
    ahi_stats.apnea_count = 0;
    ahi_stats.hypopnea_count = 0;
    ahi_stats.total_events = 0;
    ahi_stats.current_event_active = 0;
    ahi_stats.ahi_value = 0;
    
    printf("AHI Recording Started at tick: %d\r\n", system_tick_ms);
}

// 停止记录并输出结果
void AHI_Stop_Recording(void) {
    if (!ahi_stats.is_recording) return;
    
    ahi_stats.is_recording = 0;
    AHI_Calculate_Index();
    
    printf("\r\n========== AHI Report ==========\r\n");
    printf("Total Duration: %.1f hours\r\n", ahi_stats.total_duration_ms / 3600000.0f);
    printf("Apnea Count: %d\r\n", ahi_stats.apnea_count);
    printf("Hypopnea Count: %d\r\n", ahi_stats.hypopnea_count);
    printf("Total Events: %d\r\n", ahi_stats.total_events);
    printf("AHI Value: %.2f\r\n", ahi_stats.ahi_value);
    printf("Severity: %s\r\n", AHI_Get_Severity_String(ahi_stats.severity_level));
    printf("================================\r\n");
}

// 重置AHI记录
void AHI_Reset_Recording(void) {
    AHI_Stop_Recording();
    
    ahi_stats.is_recording = 0;
    ahi_stats.total_duration_ms = 0;
    ahi_stats.apnea_count = 0;
    ahi_stats.hypopnea_count = 0;
    ahi_stats.total_events = 0;
    ahi_stats.ahi_value = 0;
    ahi_stats.current_event_active = 0;
    ahi_stats.last_display_time = 0;
    ahi_stats.last_sd_log_time = 0;
    
    printf("AHI Reset Complete\r\n");
}

// 更新AHI（主函数中每秒调用）
void AHI_Update(uint16_t current_flow) {
    uint32_t current_time = system_tick_ms;
    
    // 检查是否需要采样（每秒一次）
    if (current_time - ahi_stats.last_sample_time >= AHI_SAMPLE_INTERVAL_MS) {
        ahi_stats.last_sample_time = current_time;
        
        // 更新流量缓冲区（保存最近60秒）
        ahi_stats.flow_buffer[ahi_stats.flow_buffer_idx] = current_flow;
        ahi_stats.flow_buffer_idx = (ahi_stats.flow_buffer_idx + 1) % FLOW_BUFFER_SIZE;
        
        // 更新基线
        AHI_Update_Baseline(current_flow);
        
        // 检测事件
        AHI_Check_Event(current_flow);
        
        // 更新总时长
        if (ahi_stats.is_recording) {
            ahi_stats.total_duration_ms = current_time - ahi_stats.record_start_tick;
            
            // 每5分钟计算一次AHI值
            if (ahi_stats.total_duration_ms % 300000 < AHI_SAMPLE_INTERVAL_MS) {
                AHI_Calculate_Index();
            }
        }
    }
}

// 处理AHI数据显示
void AHI_Process_And_Display(void) {
    uint32_t current_time = system_tick_ms;
    
    // 每10秒更新一次显示
    if (current_time - ahi_stats.last_display_time >= 10000) {
        ahi_stats.last_display_time = current_time;
        
        // 显示AHI信息（根据屏幕界面调整显示位置）
        // 这些printf需要根据实际的屏幕协议调整
        printf("page3.x18.val=%d\xff\xff\xff", (uint16_t)(ahi_stats.ahi_value * 10)); // AHI值（一位小数）
        
        // 显示严重程度等级
        printf("page3.x19.val=%d\xff\xff\xff", ahi_stats.severity_level);
        
        // 显示事件计数
        printf("page3.x20.val=%d\xff\xff\xff", ahi_stats.apnea_count);
        printf("page3.x21.val=%d\xff\xff\xff", ahi_stats.hypopnea_count);
        
        // 显示记录状态
        printf("page3.x22.val=%d\xff\xff\xff", ahi_stats.is_recording);
    }
}

// SD卡记录AHI数据
void SD_Log_AHI_Data(uint8_t sd_init_flag, void (*sd_write_func)(const char*)) {
    uint32_t current_time = system_tick_ms;
    
    if (!sd_init_flag || !sd_write_func) return;
    
    // 每5分钟记录一次AHI数据到SD卡
    if (current_time - ahi_stats.last_sd_log_time >= 300000) {
        ahi_stats.last_sd_log_time = current_time;
        
        char ahi_str[256];
        sprintf(ahi_str, "AHI_LOG,%d,%.2f,%d,%d,%d,%.1f,%s\r\n",
                (int)(current_time / 1000),
                ahi_stats.ahi_value,
                ahi_stats.severity_level,
                ahi_stats.apnea_count,
                ahi_stats.hypopnea_count,
                ahi_stats.total_duration_ms / 3600000.0f,
                AHI_Get_Severity_String(ahi_stats.severity_level));
        
        sd_write_func(ahi_str);
    }
}

// 获取当前AHI值
float AHI_Get_Value(void) {
    return ahi_stats.ahi_value;
}

// 获取呼吸暂停次数
uint16_t AHI_Get_Apnea_Count(void) {
    return ahi_stats.apnea_count;
}

// 获取低通气次数
uint16_t AHI_Get_Hypopnea_Count(void) {
    return ahi_stats.hypopnea_count;
}

// 获取记录状态
uint8_t AHI_Is_Recording(void) {
    return ahi_stats.is_recording;
}

// 获取记录时长（毫秒）
uint32_t AHI_Get_Record_Duration(void) {
    return ahi_stats.total_duration_ms;
}

// 手动设置基线（用于校准）
void AHI_Set_Baseline(uint16_t baseline) {
    if (baseline > 0 && baseline < 5000) {
        ahi_stats.normal_flow_baseline = baseline;
        printf("AHI Baseline set to: %d\r\n", baseline);
    }
}

// 导出完整报告
void AHI_Export_Report(char* report_buffer, uint16_t buffer_size) {
    if (!report_buffer || buffer_size < 512) return;
    
    snprintf(report_buffer, buffer_size,
        "\r\n========== AHI Complete Report ==========\r\n"
        "Recording Duration: %.1f hours (%.1f minutes)\r\n"
        "Total Events: %d\r\n"
        "  - Apnea Events: %d\r\n"
        "  - Hypopnea Events: %d\r\n"
        "AHI Index: %.2f events/hour\r\n"
        "Severity Level: %s\r\n"
        "Clinical Interpretation:\r\n"
        "  %s\r\n"
        "===========================================\r\n",
        ahi_stats.total_duration_ms / 3600000.0f,
        ahi_stats.total_duration_ms / 60000.0f,
        ahi_stats.total_events,
        ahi_stats.apnea_count,
        ahi_stats.hypopnea_count,
        ahi_stats.ahi_value,
        AHI_Get_Severity_String(ahi_stats.severity_level),
        ahi_stats.severity_level == SEVERITY_NORMAL ? "Normal - No treatment needed" :
        ahi_stats.severity_level == SEVERITY_MILD ? "Mild - Lifestyle changes may help" :
        ahi_stats.severity_level == SEVERITY_MODERATE ? "Moderate - CPAP therapy recommended" :
        "Severe - CPAP therapy strongly recommended"
    );
}