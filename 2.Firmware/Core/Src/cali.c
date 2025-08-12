#include "cali.h"

#include <stdlib.h>

void Calibration_Init(Calibration_HandleTypeDef* cali, const float* target, const float* real, uint32_t length)
{
    if (length < 2 || target == NULL || real == NULL) {
        // Handle invalid input
        return;
    }

    // Initialize the calibration structure
    cali->target = target;
    cali->real = real;
    cali->length = length;

    // Allocate memory for k and b arrays
    cali->k = (float*)malloc(sizeof(float) * (length - 1));
    cali->b = (float*)malloc(sizeof(float) * (length - 1));

    if (cali->k == NULL || cali->b == NULL) 
    { 
        // Handle memory allocation failure
        return;
    }

    // Calculate segments
    for (uint32_t i = 0; i < cali->length - 1; i++) 
    {
        float x0 = cali->real[i];
        float x1 = cali->real[i+1];
        float y0 = cali->target[i];
        float y1 = cali->target[i+1];
        cali->k[i] = (y1 - y0) / (x1 - x0);
        cali->b[i] = y0 - cali->k[i] * x0;
    }
}

float Calibration_Apply(Calibration_HandleTypeDef* cali, float real_value)
{
    // Overflow and underflow checks
    if (real_value <= cali->real[0])
        return cali->k[0] * real_value + cali->b[0];
    if (real_value >= cali->real[cali->length - 1])
        return cali->k[cali->length - 2] * real_value + cali->b[cali->length - 2];

    // Find the segment for the given real_value
    for (uint32_t i = 0; i < cali->length - 1; ++i) {
        if (real_value >= cali->real[i] && real_value <= cali->real[i+1]) {
            return cali->k[i] * real_value + cali->b[i];
        }
    }
    // In case no segment found, return an error value or handle it appropriately
    return -1;
}
