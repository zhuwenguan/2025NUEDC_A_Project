#ifndef __PID_H__
#define __PID_H__

#include "main.h"

typedef struct {
    float Kp;  // Proportional gain
    float Ki;  // Integral gain
    float Kd;  // Derivative gain
    float target; // Desired value
    float integral; // Integral term
    float integral_limit; // Limit for integral term to prevent windup
    float previous_error; // Previous error for derivative calculation
    float prev_prev_error;
} PID_HandleTypeDef;

void PID_Init(PID_HandleTypeDef *pid, float kp, float ki, float kd, float integral_limit);
void PID_SetTarget(PID_HandleTypeDef *pid, float target);
void PID_Reset(PID_HandleTypeDef *pid);
void PID_Compute(PID_HandleTypeDef *pid, float current_value, float *output);
void PID_Compute_Incremental(PID_HandleTypeDef *pid, float current_value, float *output);

#endif /* __PID_H__ */
