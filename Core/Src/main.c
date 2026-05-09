/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "dac.h"
#include "dma.h"
#include "stm32l4xx_hal_adc.h"
#include "stm32l4xx_hal_dac.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include <stdint.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
#define ADC_RESOLUTION 4096 // 12-bit ADC, values range from 0 to 4095
#define ADC_BUFFER_LENGTH 64 // number of samples to process at a time
uint16_t adc_buffer[ADC_BUFFER_LENGTH]; // Buffer to hold ADC values (output from guitar will be stored here)

// --------------------------------------------------------------------------------------
//
//                                  EFFECT SETTINGS
//
// --------------------------------------------------------------------------------------

/**
  * @attention: Effect parameters, can be adjusted to create different sounds

  * EFFECTS: 0 for passthrough, 1 for distortion, 2 for octave fuzz

  * PASSTHROUGH: no settings
  * DISTORTION:
  *   EFFECT_THRESHOLD: is the minimum value for the clipping to take place
  *   GAIN: amplifes the signal before clipping, can be adjusted to create different sounds from mild distortion to full on fuzz
*/

// general effect settings
#define EFFECT 1 // select effect to apply, via comment above
#define MIX 0.5f // mix between dry and wet signal, 0.5 for equal mix

// distortion settings
#define EFFECT_THRESHOLD 500.0f // threshold for effect to be applied, can be adjusted based on testing
#define GAIN 2.0f // gain for distortion effect, adjust based on preference

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_ADC1_Init();
  MX_USART2_UART_Init();
  MX_DAC1_Init();
  MX_TIM6_Init();

  /* USER CODE BEGIN 2 */
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, ADC_BUFFER_LENGTH); // start ADC in DMA mode, load values into adc_buffer
  HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t*)adc_buffer, ADC_BUFFER_LENGTH, DAC_ALIGN_12B_R); // start DAC in DMA mode, output values from adc_buffer to DAC channel, DAC_ALIGN_12B_R allows for easy data storage withotu bit shifting 
  HAL_TIM_Base_Start(&htim6); // start timer 6, which will trigger DAC conversions at regular intervals

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

// -----------------------------------------------------------------------------------------------------
//
//                                   EFFECT IMPLEMENTATION 
//
// -----------------------------------------------------------------------------------------------------

void ProcessAudio(uint16_t* buffer, uint32_t length) {
    for (uint32_t i = 0; i < length; i++) {
        float dry = (float)buffer[i]; // cast to a float for processing, accounts for negative values and floating points, also prevents overflows
        float wet = dry;

        #if EFFECT == 0
        // ----------------------------------
        // PASSTHROUGH
        // ----------------------------------
        #endif

        #if EFFECT == 1
        // ----------------------------------
        // DISTORTION
        // ----------------------------------

        wet = wet - 2048.0f; // DC offset removal
        wet = wet * GAIN; // apply gain for more pronounced clipping

        // clipping 
        if (wet > EFFECT_THRESHOLD) {
            wet = EFFECT_THRESHOLD;
        } else if (wet < -EFFECT_THRESHOLD) {
            wet = -EFFECT_THRESHOLD;
        }

        // DC offset restoration
        wet = wet + 2048.0f;

        // ensure sample is within 12-bit range after processing
        if (wet > 4095.0f) wet = 4095.0f;
        if (wet < 0.0f)    wet = 0.0f;

        float blend = (dry * (1.0f - MIX)) + (wet * MIX); // interpolate between dry and wet based on MIX ratio

        // save result back to buffer, cast back to uint16_t
        buffer[i] = (uint16_t)blend;
        #endif

        #if EFFECT == 2
        // ----------------------------------
        // OCTAVE FUZZ
        // ----------------------------------

        wet = wet - 2048.0f; // DC offset removal
        if (wet < 0) wet = wet * -1.0f; // make signal positive for octave effect
        wet = wet + 2048.0f; // DC offset restoration
        blend = (dry * (1.0f - MIX)) + (wet * MIX); // interpolate between dry and wet based on MIX ratio
        buffer[i] = (uint16_t)blend;
        #endif
    }

}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) {

  ProcessAudio(adc_buffer, ADC_BUFFER_LENGTH/2);

}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {

  ProcessAudio(adc_buffer + ADC_BUFFER_LENGTH/2, ADC_BUFFER_LENGTH/2);

}


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