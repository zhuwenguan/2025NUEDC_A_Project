#include "curr.h"

#include "pid.h"
#include "tim.h"
#include "dsp/filtering_functions.h"
#include "cali.h"
const float curr_cali_target[] = 
{
    0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.2, 1.4, 1.6, 2.0
};

const float curr_cali_real[] = 
{
    0.73, 0.85, 0.97, 1.06, 1.17, 1.28, 1.47, 1.72, 1.86, 2.22
};

uint32_t curr_size = 10;

PID_HandleTypeDef hpid_curr;

Calibration_HandleTypeDef curr_cali;

extern float32_t dcBlockerCoeffs[5];
float32_t currBlockerState[4];

arm_biquad_cascade_df2T_instance_f32 currBlocker;

void CURR_Init(void)
{
	Calibration_Init(&curr_cali, curr_cali_target, curr_cali_real, curr_size);
    PID_Init(&hpid_curr, 0.002f, 0.01f, 0.0f, 100.0f);
    PID_SetTarget(&hpid_curr, curr_target);

    arm_biquad_cascade_df2T_init_f32(&currBlocker, 1, dcBlockerCoeffs, currBlockerState);
}

float CurrCalc(uint16_t* buffer, uint32_t size)
{
	double sum = 0;
	float temp;
    for(uint32_t i = 0; i < size; i++)
    {
		temp = buffer[i] * 3.0f / 4095;
		arm_biquad_cascade_df2T_f32(&currBlocker, &temp, &temp, 1);
        sum += temp * temp;
    }
	sum = sqrtf(sum / size) * 4.0f;
    return sum;
}

void CURR_Update()
{	
    static uint32_t lastTime;
    if(HAL_GetTick() - lastTime < 50)
        return;

    lastTime = HAL_GetTick();

    curr = CurrCalc(curr_buffer, 1000);
	curr = Calibration_Apply(&curr_cali, curr);
    
    PID_Compute_Incremental(&hpid_curr, curr, &output_curr);
    if(output_curr >= 0.25f)
    {
        output_curr = 0.25f;
    }
    else if(output_curr <= 0.15f)
    {
        output_curr = 0.15f;
    }
	if(HAL_GetTick() > 2000)
	{
		__HAL_TIM_SetCompare(&htim8, TIM_CHANNEL_3, TIM8->ARR * (1-output_curr));
	}
}
