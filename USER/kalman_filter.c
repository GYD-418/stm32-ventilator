#include "kalman_filter.h"
#include <math.h>

// 初始化卡尔曼滤波器
void KalmanFilter_Init(KalmanFilter *kf, float dt, float process_noise, float measurement_noise) {
    // 初始化采样时间
    kf->dt = dt;
    
    // 初始化状态向量(初始值设为0)
    kf->x[0] = 0.0f;  // 位置
    kf->x[1] = 0.0f;  // 速度
    
    // 初始化状态协方差矩阵
    kf->P[0][0] = 1.0f;
    kf->P[0][1] = 0.0f;
    kf->P[1][0] = 0.0f;
    kf->P[1][1] = 1.0f;
    
    // 初始化状态转移矩阵 A
    // [1 dt]
    // [0 1 ]
    kf->A[0][0] = 1.0f;
    kf->A[0][1] = dt;
    kf->A[1][0] = 0.0f;
    kf->A[1][1] = 1.0f;
    
    // 初始化控制输入矩阵 B
    kf->B[0] = 0.5f * dt * dt;
    kf->B[1] = dt;
    
    // 初始化观测矩阵 H
    kf->H[0] = 1.0f;  // 观测位置
    kf->H[1] = 0.0f;  // 不直接观测速度
    
    // 初始化过程噪声协方差 Q
    kf->Q[0][0] = process_noise;
    kf->Q[0][1] = 0.0f;
    kf->Q[1][0] = 0.0f;
    kf->Q[1][1] = process_noise;
    
    // 初始化测量噪声协方差 R
    kf->R = measurement_noise;
}

// 更新卡尔曼滤波器
void KalmanFilter_Update(KalmanFilter *kf, float measurement, float control) {
    // 1. 预测步骤
    float x_pred[2];
    float P_pred[2][2];
    
    // 预测状态: x_pred = A*x + B*u
    x_pred[0] = kf->A[0][0] * kf->x[0] + kf->A[0][1] * kf->x[1] + kf->B[0] * control;
    x_pred[1] = kf->A[1][0] * kf->x[0] + kf->A[1][1] * kf->x[1] + kf->B[1] * control;
    
    // 预测协方差: P_pred = A*P*A' + Q
    P_pred[0][0] = kf->A[0][0] * (kf->P[0][0] * kf->A[0][0] + kf->P[0][1] * kf->A[1][0]) + 
                   kf->A[0][1] * (kf->P[1][0] * kf->A[0][0] + kf->P[1][1] * kf->A[1][0]) + 
                   kf->Q[0][0];
                   
    P_pred[0][1] = kf->A[0][0] * (kf->P[0][0] * kf->A[0][1] + kf->P[0][1] * kf->A[1][1]) + 
                   kf->A[0][1] * (kf->P[1][0] * kf->A[0][1] + kf->P[1][1] * kf->A[1][1]) + 
                   kf->Q[0][1];
                   
    P_pred[1][0] = kf->A[1][0] * (kf->P[0][0] * kf->A[0][0] + kf->P[0][1] * kf->A[1][0]) + 
                   kf->A[1][1] * (kf->P[1][0] * kf->A[0][0] + kf->P[1][1] * kf->A[1][0]) + 
                   kf->Q[1][0];
                   
    P_pred[1][1] = kf->A[1][0] * (kf->P[0][0] * kf->A[0][1] + kf->P[0][1] * kf->A[1][1]) + 
                   kf->A[1][1] * (kf->P[1][0] * kf->A[0][1] + kf->P[1][1] * kf->A[1][1]) + 
                   kf->Q[1][1];
    
    // 2. 更新步骤
    // 计算卡尔曼增益 K = P_pred*H'/(H*P_pred*H' + R)
    float HPH = kf->H[0] * (P_pred[0][0] * kf->H[0] + P_pred[0][1] * kf->H[1]) + 
                kf->H[1] * (P_pred[1][0] * kf->H[0] + P_pred[1][1] * kf->H[1]);
    float S = HPH + kf->R;
    float K[2];
    
    K[0] = (P_pred[0][0] * kf->H[0] + P_pred[0][1] * kf->H[1]) / S;
    K[1] = (P_pred[1][0] * kf->H[0] + P_pred[1][1] * kf->H[1]) / S;
    
    // 更新状态估计
    float y = measurement - (kf->H[0] * x_pred[0] + kf->H[1] * x_pred[1]);
    kf->x[0] = x_pred[0] + K[0] * y;
    kf->x[1] = x_pred[1] + K[1] * y;
    
    // 更新协方差矩阵
    float IKH[2][2] = {
        {1.0f - K[0] * kf->H[0], -K[0] * kf->H[1]},
        {-K[1] * kf->H[0], 1.0f - K[1] * kf->H[1]}
    };
    
    // P = (I - K*H) * P_pred
    kf->P[0][0] = IKH[0][0] * P_pred[0][0] + IKH[0][1] * P_pred[1][0];
    kf->P[0][1] = IKH[0][0] * P_pred[0][1] + IKH[0][1] * P_pred[1][1];
    kf->P[1][0] = IKH[1][0] * P_pred[0][0] + IKH[1][1] * P_pred[1][0];
    kf->P[1][1] = IKH[1][0] * P_pred[0][1] + IKH[1][1] * P_pred[1][1];
}

