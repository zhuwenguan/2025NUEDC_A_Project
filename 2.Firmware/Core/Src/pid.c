#include "pid.h"

void PID_Init(PID_HandleTypeDef *pid, float kp, float ki, float kd, float integral_limit)
{
    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;
    pid->previous_error = 0.0f;
    pid->integral = 0.0f;
    pid->integral_limit = integral_limit;
}

void PID_SetTarget(PID_HandleTypeDef *pid, float target)
{
    pid->target = target;
}

void PID_Reset(PID_HandleTypeDef *pid)
{
    pid->integral = 0.0f;
    pid->previous_error = 0.0f;
}

void PID_Compute(PID_HandleTypeDef *pid, float current_value, float *output)
{
    float error = pid->target - current_value;
    pid->integral += error;
    // Limit integral to prevent windup
    if (pid->integral > pid->integral_limit) {
        pid->integral = pid->integral_limit;
    } else if (pid->integral < -pid->integral_limit) {
        pid->integral = -pid->integral_limit;
    }
    float derivative = error - pid->previous_error;
    *output = pid->Kp * error + pid->Ki * pid->integral + pid->Kd * derivative;
    pid->previous_error = error;
}

// 增量式PID计算函数
void PID_Compute_Incremental(PID_HandleTypeDef *pid, float current_value, float *output)
{
    float error = pid->target - current_value;
    float delta_output = pid->Kp * (error - pid->previous_error)
                        + pid->Ki * error
                        + pid->Kd * (error - 2 * pid->previous_error + pid->prev_prev_error);
    *output += delta_output;
    pid->prev_prev_error = pid->previous_error;
    pid->previous_error = error;
}
