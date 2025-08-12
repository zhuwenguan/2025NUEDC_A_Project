#ifndef __SPWM_H
#define __SPWM_H

#include "main.h"

typedef struct
{
    uint16_t* array;
    uint32_t size;
    uint32_t max_value;
    float phase;
    uint32_t update_max_value;
} SPWM_HandleTypeDef;

void SPWM_Init(SPWM_HandleTypeDef* hspwm, uint32_t max_value, uint32_t freq, float phase);
void SPWM_UpdateStart(uint32_t update_max_value, uint32_t updateFreq);
void SPWM_Update(void);	

#endif
