#include "spwm.h"
#include "tim.h"

#include "dsp/fast_math_functions.h"

#include <stdlib.h>

void SPWM_Init(SPWM_HandleTypeDef* hspwm, uint32_t max_value, uint32_t freq, float phase)
{
	hspwm->size = PWM_FREQ / freq;
	uint32_t size = hspwm->size;
	hspwm->array = (uint16_t*)malloc((PWM_FREQ / 20) * sizeof(uint16_t));
	hspwm->max_value = max_value;
	hspwm->phase = phase;
	for (uint32_t i = 0; i < size; i++)
    {
		uint32_t min = 0.01 * max_value;
		uint32_t max = 0.99 * max_value;
		uint32_t temp = max_value * (0.5f - 0.5f * arm_cos_f32(2.0f * PI * i / size + phase * 2 * PI));
		if(temp > max)
			temp = max;
		if(temp < min)
			temp = min;
        hspwm->array[i] = temp;
    }
}

extern SPWM_HandleTypeDef hspwm_u, hspwm_v, hspwm_w;

void SPWM_UpdateStart(uint32_t update_max_value, uint32_t updateFreq)
{
	if(updateFlag == 1)
		return;
	
	hspwm_u.update_max_value = update_max_value;
	hspwm_u.size = PWM_FREQ / updateFreq;
	hspwm_v.update_max_value = update_max_value;
	hspwm_v.size = PWM_FREQ / updateFreq;
	hspwm_w.update_max_value = update_max_value;
	hspwm_w.size = PWM_FREQ / updateFreq;
	updateFlag = 1;
}

void SPWM_Update()
{
	if(updateFlag == 0)
		return;
    
	// 更新三个相的SPWM波形
	SPWM_HandleTypeDef* handles[3] = {&hspwm_u, &hspwm_v, &hspwm_w};
	float phases[3] = {hspwm_u.phase, hspwm_v.phase, hspwm_w.phase};
	static uint32_t lastFreq = 50;
	for (int h = 0; h < 3; h++) {
		SPWM_HandleTypeDef* hspwm = handles[h];
		uint32_t size = hspwm->size;
		uint32_t max_value = hspwm->update_max_value;
		float phase = phases[h];
		if (hspwm->array == NULL) continue;
		for (uint32_t i = 0; i < size; i++) {
			uint32_t min = 0.01 * max_value;
			uint32_t max = 0.99 * max_value;
			uint32_t temp = max_value * (0.5f - 0.5f * arm_cos_f32(2.0f * PI * i / size + phase * 2 * PI));
			if(temp > max)
				temp = max;
			if(temp < min)
				temp = min;
			hspwm->array[i] = temp;
		}
	}
	if(lastFreq != freq)
	{
		HAL_TIM_PWM_Stop_DMA(&htim1, TIM_CHANNEL_1);
		HAL_TIM_PWM_Stop_DMA(&htim1, TIM_CHANNEL_2);
		HAL_TIM_PWM_Stop_DMA(&htim1, TIM_CHANNEL_3);
		HAL_Delay(1);
		HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, (uint32_t*)hspwm_u.array, hspwm_u.size);
		HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_2, (uint32_t*)hspwm_v.array, hspwm_v.size);
		HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_3, (uint32_t*)hspwm_w.array, hspwm_w.size);
	}
	lastFreq = freq;
	updateFlag = 0;
}



