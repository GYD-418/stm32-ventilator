#ifndef KALMAN_FILTER_H
#define KALMAN_FILTER_H

#include "stm32f4xx.h"                  // Device header
#include "sys.h"

// 卡尔曼滤波器结构体定义
typedef struct {
    // 状态向量 [x, v] 位置和速度
    float x[2];
    
    // 状态协方差矩阵
    float P[2][2];
    
    // 状态转移矩阵
    float A[2][2];
    
    // 控制输入矩阵
    float B[2];
    
    // 观测矩阵
    float H[2];
    
    // 过程噪声协方差
    float Q[2][2];
    
    // 测量噪声协方差
    float R;
    
    // 采样时间(秒)
    float dt;
} KalmanFilter;

// 函数声明
void KalmanFilter_Init(KalmanFilter *kf, float dt, float process_noise, float measurement_noise);
void KalmanFilter_Update(KalmanFilter *kf, float measurement, float control);

#endif /* KALMAN_FILTER_H */

