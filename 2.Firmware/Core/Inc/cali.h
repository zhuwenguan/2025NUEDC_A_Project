#ifndef __CALI_H
#define __CALI_H

#include "main.h"

typedef struct {
    const float* target;    // Target values array
    const float* real;      // Real measured values array
    uint32_t length;        // Points
    float* k;               // Slopes of each segment array, length is length-1
    float* b;               // Intercepts of each segment array, length is length-1
} Calibration_HandleTypeDef;

void Calibration_Init(Calibration_HandleTypeDef* cali, const float* target, const float* real, uint32_t length);
float Calibration_Apply(Calibration_HandleTypeDef* cali, float real_value);

#endif
