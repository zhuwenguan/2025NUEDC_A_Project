#include "volt.h"
#include "pid.h"
#include "spwm.h"
#include "cali.h"
#include "cali_data.h"

#include "dsp/filtering_functions.h"

PID_HandleTypeDef hpid_volt;

float32_t dcBlockerCoeffs[5] = { 1.0f, -1.0f, 0.0f, 0.995f, 0.0f };

Calibration_HandleTypeDef curr_volt_cali;
Calibration_HandleTypeDef freq_volt_cali;

// 滤波器状态 (每阶需要4个state)
float32_t dcBlockerState[4];

// 定义biquad滤波器实例
arm_biquad_cascade_df2T_instance_f32 dcBlocker;

void VOLT_Init(void)
{
    PID_Init(&hpid_volt, 0.001f, 0.01f, 0.0f, 100.0f);
    PID_SetTarget(&hpid_volt, target);
	
    arm_biquad_cascade_df2T_init_f32(&dcBlocker, 1, dcBlockerCoeffs, dcBlockerState);
	
	Calibration_Init(&curr_volt_cali, curr_volt_target, curr_volt_real, curr_volt_size);
	Calibration_Init(&freq_volt_cali, freq_volt_target, freq_volt_real, freq_volt_size);
}

float RMS(uint16_t* buffer, uint32_t size)
{
    float sum = 0;
	float volt;
    for(uint32_t i = 0; i < size; i++)
    {
		volt = buffer[i] * 2.985f / 4095 / 0.0214238f;
		arm_biquad_cascade_df2T_f32(&dcBlocker, &volt, &volt, 1);
        sum += volt * volt;
    }
	sum = sqrtf(sum / size);
    return sum;
}



void VOLT_Update()
{
	float curr_coeff = 1.0f;
	float freq_coeff = 1.0f;
	static uint32_t lastTime;
	if(HAL_GetTick() - lastTime < 100)
		return;
	
	
	lastTime = HAL_GetTick();
	
	curr_coeff = Calibration_Apply(&curr_volt_cali, curr);
//	freq_coeff = Calibration_Apply(&freq_volt_cali, freq);
	rms = RMS(volt_buffer, 1000) / curr_coeff / freq_coeff;
    
    PID_Compute_Incremental(&hpid_volt, rms, &output);
	if(output >= 1.05f)
	{
		output = 1.05f;
	}
	if(output < 0.943f && curr >= 0.8f)
	{
		output = 0.943f;
	}
	
	SPWM_UpdateStart(output * PWM_MAX, freq);
}


