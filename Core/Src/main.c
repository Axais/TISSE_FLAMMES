	/* USER CODE BEGIN Header */
	/**
	  ******************************************************************************
	  * @file           : main.c
	  * @brief          : Main program body
	  ******************************************************************************
	  */
	/* USER CODE END Header */
	/* Includes ------------------------------------------------------------------*/
	#include "main.h"
	#include "dma.h"
	#include "tim.h"
	#include "usart.h"
	#include "gpio.h"
	#include<stdlib.h>

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
	uint32_t pos_actuelle_X = 0;
	uint32_t pos_actuelle_Y = 0;
	const uint32_t COURSE_MAX = 2000; // Nombre de pas pour traverser le portique
	/* USER CODE END PV */

	/* Private function prototypes -----------------------------------------------*/
	void SystemClock_Config(void);
	/* USER CODE BEGIN PFP */
	void deplacement_X(uint8_t sens, uint32_t nb_pas);
	void deplacement_Y(uint8_t sens, uint32_t nb_pas);
	void delay_us(uint16_t us);
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
	  HAL_Init();
	  SystemClock_Config();

	  /* Initialize all configured peripherals */
	  MX_GPIO_Init();
	  MX_DMA_Init();
	  MX_TIM6_Init();
	  MX_USART1_UART_Init();

	  /* USER CODE BEGIN 2 */
	  HAL_TIM_Base_Start(&htim6);

	  // Démarrage de la réception DMX optimisée
	  dmx_start();
	  deplacement_X(1,600);
	  deplacement_Y(1,600);
	  /* USER CODE END 2 */

	  /* Infinite loop */
	  /* USER CODE BEGIN WHILE */
	  while (1)
	  {


		  // --- PROTECTION CONTRE LE BLOCAGE ---
		  // Si l'UART sature (Overrun), on nettoie et on relance le moteur DMX
		  if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_ORE)) {
			  __HAL_UART_CLEAR_OREFLAG(&huart1);
			  dmx_start();
		  }

		  // --- TEST VISUEL ---
		  // canal[1] est le Dimmer. Si > 0, on allume la LED LD2 (PA5)
		  if (canal[1] > 0) {
			  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
		  } else {
			  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
		  }

		  // --- SÉCURITÉ ET MOUVEMENT ---
		  if (canal[1] > 150) // Condition de sécurité maximale
		  {
			  uint32_t cible_X = (uint32_t)((canal[2] * COURSE_MAX) / 255);
			  uint32_t cible_Y = (uint32_t)((canal[3] * COURSE_MAX) / 255);

			  // Axe X
			  if (cible_X != pos_actuelle_X) {
				  uint8_t sens_X = (cible_X > pos_actuelle_X) ? 1 : 0;
				  uint32_t nb_pas_X = abs((int32_t)cible_X - (int32_t)pos_actuelle_X);
				  deplacement_X(sens_X, nb_pas_X);
				  pos_actuelle_X = cible_X;
			  }

			  // Axe Y
			  if (cible_Y != pos_actuelle_Y) {
				  uint8_t sens_Y = (cible_Y > pos_actuelle_Y) ? 1 : 0;
				  uint32_t nb_pas_Y = abs((int32_t)cible_Y - (int32_t)pos_actuelle_Y);
				  deplacement_Y(sens_Y, nb_pas_Y);
				  pos_actuelle_Y = cible_Y;
			  }
		  }
	  }
	  /* USER CODE END WHILE */

	  /* USER CODE BEGIN 3 */
	  /* USER CODE END 3 */
	}

	/**
	  * @brief System Clock Configuration
	  */
	void SystemClock_Config(void)
	{
	  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
	  {
		Error_Handler();
	  }

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
	void deplacement_X(uint8_t sens, uint32_t nb_pas) {
		HAL_GPIO_WritePin(DIR_PIN_GPIO_Port, DIR_PIN_Pin, (sens == 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
		for(uint32_t i = 0; i < nb_pas; i++) {
			HAL_GPIO_WritePin(STEP_PIN_GPIO_Port, STEP_PIN_Pin, GPIO_PIN_SET);
			HAL_Delay(2);
			HAL_GPIO_WritePin(STEP_PIN_GPIO_Port, STEP_PIN_Pin, GPIO_PIN_RESET);
			HAL_Delay(2);
		}
	}

	void deplacement_Y(uint8_t sens, uint32_t nb_pas) {
		HAL_GPIO_WritePin(DIR_PIN_Y1_GPIO_Port, DIR_PIN_Y1_Pin, (sens == 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
		HAL_GPIO_WritePin(DIR_PIN_Y2_GPIO_Port, DIR_PIN_Y2_Pin, (sens == 1) ? GPIO_PIN_RESET : GPIO_PIN_SET);
		for(uint32_t i = 0; i < nb_pas; i++) {
			HAL_GPIO_WritePin(STEP_PIN_Y1_GPIO_Port, STEP_PIN_Y1_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(STEP_PIN_Y2_GPIO_Port, STEP_PIN_Y2_Pin, GPIO_PIN_SET);
			HAL_Delay(2);
			HAL_GPIO_WritePin(STEP_PIN_Y1_GPIO_Port, STEP_PIN_Y1_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(STEP_PIN_Y2_GPIO_Port, STEP_PIN_Y2_Pin, GPIO_PIN_RESET);
			HAL_Delay(2);
		}
	}

	void delay_us(uint16_t us) {
		__HAL_TIM_SET_COUNTER(&htim6, 0);
		while (__HAL_TIM_GET_COUNTER(&htim6) < us);
	}
	/* USER CODE END 4 */

	void Error_Handler(void)
	{
	  __disable_irq();
	  while (1) {}
	}

	#ifdef  USE_FULL_ASSERT
	void assert_failed(uint8_t *file, uint32_t line)
	{
	}
	#endif /* USE_FULL_ASSERT */
