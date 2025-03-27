/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
ADC_HandleTypeDef hadc1;

TIM_HandleTypeDef htim7;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM7_Init(void);
const char *char_sounds[] = {
    "._",    // A
    "_...",  // B
    "_._.",  // C
    "_..",   // D
    ".",     // E
    ".._.",  // F
    "__.",   // G
    "....",  // H
    "..",    // I
    ".___",  // J
    "_._",   // K
    "._..",  // L
    "__",    // M
    "_.",    // N
    "___",   // O
    ".__.",  // P
    "__._",  // Q
    "._.",   // R
    "...",   // S
    "_",     // T
    ".._",   // U
    "..._",  // V
    ".__",   // W
    "_.._",  // X
    "_.__",  // Y
    "__..",  // Z
    " ",     // Espace
    " ",     // Autres caractÃ¨res
};

char decode_morse_letter(const char morse_code) {
    for (int i = 0; i < 28; i++) {
        if (strcmp(morse_code, char_sounds[i]) == 0) {
            if (i < 26) {
                return 'A' + i; // A ASCII 65, B ASCII 66...
            } else if (i == 26) {
                return ' '; //Espace
            } else {
                return '?';
            }
        }
    }
}

char* decode_morse(const char *morse_code) {
    int i = 0;
    char *string = malloc(strlen(morse_code) + 1); // Allocate memory for decoded string
    if (string == NULL) {
        return NULL; // Memory allocation failed
    }

    while (morse_code[i] != '\0') { // Fix '\0' termination
        string[i] = decode_morse_letter(morse_code[i]);
        i++;
    }
    string[i] = '\0'; // Null-terminate the string
    return string;
}

int adc_value;
char buffer[50];


// ...

#include "main.h"
#include <stdio.h>

GPIO_TypeDef *port_target = GPIOA;//par défaut le port du microphone
uint16_t *pin_target = GPIO_PIN_0;//par défaut le pin du microphone
//ces variable alternerons entre les ports et pins du micro et du récépteur a chaque déclencgement d'intérruption du boutton user

// ...
// max_buzzer = 3130;
// min_buzzer = 3030;
// max_led = 4000;

int seuil_max = 3130;//par défaut en mode micro
int seuil_min = 3030;

int mode =0;

int main(void) {
  // ...
    // ...
	    /* Initialize HAL */
	    HAL_Init();

	    /* Configure the system clock */
	    SystemClock_Config();

	    /* Initialize peripherals */
	    MX_GPIO_Init();
	    MX_ADC1_Init();
	    MX_USART2_UART_Init();
      /* Start ADC conversion */
	    int timer_start;
	    int duration;
	    int k=0;
	    int en_cours;
	    int temps_pause;
	    char code_morse[150];

  while (1) {

       if (HAL_ADC_Start(&hadc1) != HAL_OK)
       {
           Error_Handler();
       }

       /* Wait for ADC conversion to complete */
       if (HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) != HAL_OK)
       {
           Error_Handler();
       }

       adc_value= HAL_ADC_GetValue(&hadc1);
       if (adc_value > seuil_max) {//son ou bien led allumé detecté
    	en_cours = 1;
       	timer_start = HAL_GetTick(); // début compteur
        sprintf(buffer, "Start listening %lu \n", adc_value);
       	HAL_UART_Transmit(&huart2, buffer, strlen(buffer), 1000);
        while (adc_value > seuil_max || adc_value <= seuil_min) {//condition pour vérifier que le son ou bien led est toujours actif
        	HAL_Delay(51.4);
            if (HAL_ADC_Start(&hadc1) != HAL_OK) {
                Error_Handler();
            }

            /* Wait for ADC conversion to complete */
            if (HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) != HAL_OK) {
                Error_Handler();
            }

    		adc_value = HAL_ADC_GetValue(&hadc1);
        }
		sprintf(buffer, "valeur %lu \n", adc_value);
		HAL_UART_Transmit(&huart2, buffer, strlen(buffer), 1000);
       	duration = HAL_GetTick() - timer_start; // Calculate the duration
    	temps_pause = HAL_GetTick();
       	sprintf(buffer, "Duration %lu \n", duration);
       	HAL_UART_Transmit(&huart2, buffer, strlen(buffer), 1000);

        if (duration >= 60 && duration <= 400){ // correspond a un point
     	   HAL_UART_Transmit(&huart2, ".\n", 2, 1000);
     	   code_morse[k] = '.';
     	   k++;
        }
        else if (duration > 400  && duration <= 850){ //correspond a un tiret
     	   HAL_UART_Transmit(&huart2, "_\n", 2, 1000);
     	   code_morse[k] = '_';
     	   k++;
        }
       }

       else {

    	   if (en_cours == 1 && (HAL_GetTick() - temps_pause > 2000)){//correspond a un silence sépérieur a 2s aprés l'écoute
    		   char val[100];
    		   strncpy(val, code_morse, k);//fin de la récéption du code morse
    		   val[k] = '\0';
    		   sprintf(buffer, "code morse : %s\n", val);
    		   en_cours = 0;
    		   k=0;
    		   HAL_UART_Transmit(&huart2, buffer, strlen(buffer), 1000);
    	   }
       }
 	  HAL_Delay(51.4);
    //...
  }

  // ...
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 16000-1;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 65535;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
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

#ifdef  USE_FULL_ASSERT
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
