#include "PID.h"
#include <math.h>

/**
 * @brief 标准PID控制器初始化
 * @param pid: PID控制器结构体指针
 * @param Kp: 比例系数（快速响应误差）
 * @param Ki: 积分系数（消除静态误差）
 * @param Kd: 微分系数（抑制超调，提升稳定性）
 * @param integral_limit: 积分项限幅（防止积分饱和）
 * @param output_limit: PID输出限幅（匹配执行器能力）
 */
void PID_Init(PID_Controller *pid, float Kp, float Ki, float Kd,
             float integral_limit, float output_limit) {
    // 初始化PID核心参数
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    
    // 初始化状态变量（避免初始值混乱）
    pid->target = 0.0f;       // 初始目标值设为0
    pid->last_error = 0.0f;   // 初始误差设为0
    pid->integral = 0.0f;     // 初始积分项设为0
    
    // 初始化限幅参数（保障系统安全）
    pid->integral_limit = integral_limit;
    pid->output_limit = output_limit;
}

/**
 * @brief 设置PID控制器的目标值（如目标压力、目标转速）
 * @param pid: PID控制器结构体指针
 * @param target: 新的控制目标值
 */
void PID_SetTarget(PID_Controller *pid, float target) {
    pid->target = target;
    // 重置历史状态（目标突变时避免积分项和误差累积导致的输出波动）
    pid->last_error = 0.0f;
    pid->integral = 0.0f;
}

/**
 * @brief 计算标准PID输出（位置式PID算法）
 * @param pid: PID控制器结构体指针
 * @param current_value: 当前反馈值（如当前压力、当前转速）
 * @param dt: 控制周期（单位：秒，与主循环周期一致，如0.2s）
 * @return 经过限幅后的PID输出值（如PWM占空比、电压等）
 */
float PID_Compute(PID_Controller *pid, float current_value, float dt) {
    float error, derivative, output;  // 误差、微分项、最终输出
    
    // 1. 计算当前误差（目标值 - 反馈值）
    error = pid->target - current_value;
    
    // 2. 计算积分项（积分 = 积分 + 误差 * 积分系数 * 控制周期）
    // 先判断误差方向，避免积分项反向累积（优化积分饱和）
    if ((pid->integral < pid->integral_limit && pid->integral > -pid->integral_limit) ||
        (error > 0 && pid->integral < 0) || (error < 0 && pid->integral > 0)) {
        pid->integral += error * pid->Ki * dt;
    }
    
    // 3. 积分项限幅（强制限制在设定范围内，防止积分饱和）
    if (pid->integral > pid->integral_limit) {
        pid->integral = pid->integral_limit;
    } else if (pid->integral < -pid->integral_limit) {
        pid->integral = -pid->integral_limit;
    }
    
    // 4. 计算微分项（微分 = （当前误差 - 上一次误差） / 控制周期，反映误差变化趋势）
    derivative = (error - pid->last_error) / dt;
    
    // 5. 计算PID总输出（输出 = 比例项 + 积分项 + 微分项）
    output = (error * pid->Kp) + pid->integral + (derivative * pid->Kd);
    
    // 6. 输出限幅（匹配执行器能力，如PWM占空比0-100）
    if (output > pid->output_limit) {
        output = pid->output_limit;
    } else if (output < 0) {  // 若执行器仅支持正输出（如风机），下限设为0
        output = 0;
    }
    
    // 7. 保存当前误差为下一次计算的“上一次误差”
    pid->last_error = error;
    
    return output;
}
