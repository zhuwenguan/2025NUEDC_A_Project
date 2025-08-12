#ifndef __CALI_DATA_H__
#define __CALI_DATA_H__

#include "main.h"

float curr_volt_target[] = 
{
    1.0,1.0,1.0,1.0,1.0009,1.00020,1.0011,1.0012,1.0013,1.00246,1.0049,1.0060  
};

float curr_volt_real[] = 
{
    0.5,0.6,0.7,0.8,0.9,1.0,1.2,1.5,1.7,2.0,2.2,2.5
};

uint32_t curr_volt_size = 12;

float freq_volt_target[] = 
{
    0.947779, 0.978337, 0.991920, 1.000000, 1.001902, 1.003812, 1.002856, 1.002538, 1.001000
};

float freq_volt_real[] = 
{
    20,30,40,50,60,70,80,90,100
};

uint32_t freq_volt_size = 9;

#endif
