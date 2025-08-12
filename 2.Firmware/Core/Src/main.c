/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "ltdc.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "spwm.h"
#include "volt.h"
#include "curr.h"
#include "ili9341.h"
#include "lcd.h"
#include "stm32f429i_discovery_lcd.h"
#include <string.h>
#include <stdio.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

uint8_t updateFlag;
SPWM_HandleTypeDef hspwm_u, hspwm_v, hspwm_w;
uint16_t volt_buffer[1000];
uint16_t curr_buffer[1000];

uint16_t frameBuffer[240*160];

float rms;
float output = 1.0f;
float target = 32.0f;
float curr_target = 1.0f;
uint32_t freq = 50;
float curr;
float output_curr = 0.15f;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void KeyScan()
{
	static uint32_t lastTime, addTime;
	static uint32_t cnt = 0;
	static uint32_t longPress;
	if(HAL_GetTick() - lastTime >= 10)
	{
		lastTime = HAL_GetTick();
		if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET)
		{
			if(cnt < 2)
			{
				cnt++;
			}
			if(cnt == 2)
			{
				longPress++;
				if(longPress >= 200)
				{
					if(HAL_GetTick() - addTime >= 100)
					{
						addTime = HAL_GetTick();
						if(freq < 150)
							freq++;
					}
				}
			}
		}
		else
		{
			if(cnt == 2)
			{
				cnt = 3;
				freq++;
				if(freq > 150)
					freq = 20;
			}
			else
			{
				cnt = 0;
				longPress = 0;
			}
		}
	}
}

void LCD_Init()
{
	ili9341_Init();
	BSP_LCD_SetTextColor(0xFFFF);
	BSP_LCD_SetFont(&Font12);
}

void LCD_Refresh()
{
	static uint32_t lastTime;
	if(HAL_GetTick() - lastTime < 200)
		return;
	
	lastTime = HAL_GetTick();
	char str[64];
	snprintf(str, 64, "Volt: %.2f", rms);
	BSP_LCD_DisplayStringAt(0, 0, (uint8_t*)str, LEFT_MODE);
//	snprintf(str, 64, "Current: %.2f", curr);
//	BSP_LCD_DisplayStringAt(0, 14, (uint8_t*)str, LEFT_MODE);
	snprintf(str, 64, "Freq: %d Hz", freq);
	BSP_LCD_DisplayStringAt(0, 14, (uint8_t*)str, LEFT_MODE);
//	snprintf(str, 64, "SPWM Rate: %.2f", output);
//	BSP_LCD_DisplayStringAt(0, 42, (uint8_t*)str, LEFT_MODE);
//	snprintf(str, 64, "BOOST Duty: %.2f", output_curr);
//	BSP_LCD_DisplayStringAt(0, 56, (uint8_t*)str, LEFT_MODE);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM1_Init();
  MX_TIM8_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  MX_ADC2_Init();
  MX_LTDC_Init();
  MX_SPI5_Init();
  /* USER CODE BEGIN 2 */
  SPWM_Init(&hspwm_u, PWM_MAX, freq, 0.0f);
  SPWM_Init(&hspwm_v, PWM_MAX, freq, 0.3333333f);
  SPWM_Init(&hspwm_w, PWM_MAX, freq, 0.6666666f);
  
  HAL_TIM_Base_Start(&htim8);
  __HAL_TIM_SetCompare(&htim8, TIM_CHANNEL_3, TIM8->ARR+1);
  HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_3);
  
  HAL_TIM_Base_Start(&htim1);
  HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, (uint32_t*)hspwm_u.array, hspwm_u.size);
  HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_2, (uint32_t*)hspwm_v.array, hspwm_v.size);
  HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_3, (uint32_t*)hspwm_w.array, hspwm_w.size);
  
  HAL_TIM_Base_Start(&htim2);
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)volt_buffer, 1000);
  HAL_ADC_Start_DMA(&hadc2, (uint32_t*)curr_buffer, 1000);
  
  LCD_Init();
  VOLT_Init();
  CURR_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  VOLT_Update();
	  CURR_Update();
	  SPWM_Update();
	  LCD_Refresh();
	  KeyScan();
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 180;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
