#ifndef __PID_H__
#define __PID_H__

// 标准位置式PID控制器结构体（移除模糊相关参数）
typedef struct {
    // 1. PID核心参数
    float Kp;               // 比例系数
    float Ki;               // 积分系数
    float Kd;               // 微分系数
    
    // 2. 目标与状态变量
    float target;           // 控制目标值（如目标压力）
    float last_error;       // 上一次误差（用于计算微分项）
    float integral;         // 积分项累积值（防止积分饱和）
    
    // 3. 限幅参数（保障系统稳定）
    float integral_limit;   // 积分项限幅（避免积分饱和）
    float output_limit;     // PID输出限幅（匹配执行器范围，如PWM占空比0-100）
} PID_Controller;

// 4. PID控制接口函数
void PID_Init(PID_Controller *pid, float Kp, float Ki, float Kd,
             float integral_limit, float output_limit);  // PID初始化
void PID_SetTarget(PID_Controller *pid, float target);   // 设置控制目标
float PID_Compute(PID_Controller *pid, float current_value, float dt);  // 计算PID输出

#endif
