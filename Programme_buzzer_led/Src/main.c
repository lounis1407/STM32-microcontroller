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
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
const char *char_sounds[] = { //tableau avec toutes le codes morses
    "._",   // A
    "_...", // B
    "_._.", // C
    "_..",  // D
    ".",    // E
    ".._.", // F
    "__.",  // G
    "....", // H
    "..",   // I
    ".___", // J
    "_._",  // K
    "._..", // L
    "__",   // M
    "_.",   // N
    "___",  // O
    ".__.", // P
    "__._", // Q
    "._.",  // R
    "...",  // S
    "_",    // T
    ".._",  // U
    "..._", // V
    ".__",  // W
    "_.._", // X
    "_.__", // Y
    "__..", // Z
    " ",    // Espace
    " ",    // Autres caractères
};
int mode = 1; // sert a determiner si c'est en mode buzzer(0) ou bien en mode led(1), par défaut on choisis le mode led
//la valeur de mode change entre 1 et 0 par l'intérruption du boutton user
GPIO_TypeDef *port_target = LED_GPIO_Port;//par défaut le port de la led
uint16_t *pin_target = LED_Pin;//par défaut le pin de la led


#include <string.h>
char *convertToMorse(const char *str)
{// fonction qui sert a convertir une chaine de caractéres en code morse
  int length = strlen(str);
  char *result = (char *)malloc(length * 5 + 1); // 5 est la longueur maximale d'un caractère en morse, +1 pour le caractère de fin de chaîne
  int index = 0;

  for (int i = 0; i < length; i++)
  {
    char c = toupper(str[i]); // Convertir le caractère en majuscule
    if (c >= 'A' && c <= 'Z')
    {
      const char *morse = char_sounds[c - 'A'];
      int morseLength = strlen(morse);
      strncpy(result + index, morse, morseLength);
      index += morseLength;
      result[index++] = ' ';
    }
    else if (c == ' ')
    {
      result[index++] = ' ';
    }
    else
    {
      const char *morse = char_sounds[26];
      int morseLength = strlen(morse);
      strncpy(result + index, morse, morseLength);
      index += morseLength;
      result[index++] = ' ';
    }
  }

  result[index] = '\0'; // Ajouter le caractère de fin de chaîne

  return result;
}

char *convertToText(const char *morse)
{
  int length = strlen(morse);
  char *result = (char *)malloc(length + 1); // +1 pour le caractère de fin de chaîne
  int index = 0;

  char *token = strtok(morse, " ");
  while (token != NULL)
  {
    if (strcmp(token, " ") == 0)
    {
      result[index++] = ' ';
    }
    else
    {
      int i = 0;
      while (strcmp(char_sounds[i], token) != 0 && i < 27)
      {
        i++;
      }
      if (i < 27)
      {
        result[index++] = 'A' + i;
      }
    }
    token = strtok(NULL, " ");
  }

  result[index] = '\0'; // Ajouter le caractère de fin de chaîne

  return result;
}

void play_sound_or_led(char *sound_sequence)
{
  sound_sequence = convertToMorse(sound_sequence);
  HAL_UART_Transmit(&huart2, (uint8_t *)sound_sequence, strlen(sound_sequence), 1000);
  for (int i = 0; sound_sequence[i] != '\0'; i++)
  {
	HAL_GPIO_WritePin((GPIO_TypeDef*)port_target, (uint16_t)pin_target, GPIO_PIN_SET); // Activer le buzzer/led
    if (sound_sequence[i] == '.')
      HAL_Delay(200); // Durée pour une "point"
    else if (sound_sequence[i] == '_')
      HAL_Delay(600);                                           // Durée pour un "tiret"
    HAL_GPIO_WritePin((GPIO_TypeDef*)port_target, (uint16_t)pin_target, GPIO_PIN_RESET); // Désactiver le buzzer/led
    HAL_Delay(100);                                             // Pause entre les sons
  }
}

void read_uart_content()
{
  uint8_t ch = 0;
  uint8_t buffer[100]; // Create a buffer to store received data
  int index = 0;       // Initialize index variable

  while (1)
  {
    HAL_UART_Receive(&huart2, &ch, 1, HAL_MAX_DELAY); // Pass the address of 'ch' to HAL_UART_Receive
    if (ch == '\n' || ch == '\r')
    {        // Check for end of line characters
      break; // Exit the loop if end of line is received
    }
    buffer[index] = ch; // Store received data in buffer
    index++;            // Increment index
  }

  uint8_t val[index + 1];                      // Create a new variable 'val' with size equal to 'index'
  strncpy((char *)val, (char *)buffer, index); // Copy 'index' number of characters from 'buffer' to 'val'
  val[index] = '\0';

  HAL_UART_Transmit(&huart2, val, strlen((char *)val), 1000); // Transmit the received data
  play_sound_or_led((char *)val);                                    // code morse envoyé au buzzer/led
}

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
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);// on désavtive(met a 0 leurs pins) la led et le buzzer au tout début.
  HAL_GPIO_WritePin(buzzer_GPIO_Port, buzzer_Pin, GPIO_PIN_RESET);
  while (1)
  {
    /* USER CODE END WHILE */
	  read_uart_content();
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
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

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED_Pin|buzzer_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : boutton_Pin */
  GPIO_InitStruct.Pin = boutton_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(boutton_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : buzzer_Pin */
  GPIO_InitStruct.Pin = buzzer_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(buzzer_GPIO_Port, &GPIO_InitStruct);

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
