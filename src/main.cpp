/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#define NO_DISPLAY 		0  // состояния флага what_time_flag
#define CURRENT_TIME	1
#define ON_TIME			2
#define OFF_TIME		3
#define NO_DIG			0  // состояния флага what_dig_flag
#define HOURS_DIG		1
#define TEN_MINUTES_DIG	2
#define MINUTES_DIG		3

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;

TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

/* USER CODE BEGIN PV */
uint8_t what_time_flag = ON_TIME, what_dig_flag = HOURS_DIG;  //глобальные флаги

class Time  // класс Time используется как обертка над существующей структурой RTC_TimeTypeDef. Класс был написан для тренировка использования классов в программировании микроконтроллеров.
{           //В классе реализованы: сохранение инварианта временного стандарта и функции получения часов и минут и их изменение.
private:
	RTC_TimeTypeDef data;
	void rightTime()
	{
		if(data.Seconds > 59)
			data.Seconds -= 60;
		if(data.Minutes > 59)
			data.Minutes -= 60;
		if(data.Hours > 23)
		{
			if(data.Hours > 100)
				data.Hours = 23;
			else
				data.Hours = 0;
		}
	}
public:
	Time (): data{0}
	{
	}
	Time (uint8_t h, uint8_t m): data{0}
	{
		data.Hours = h;
		data.Minutes = m;
	}
	uint8_t GetMinutes()
	{
		return data.Minutes;
	}
	uint8_t GetHours()
	{
		return data.Hours;
	}
	void addMinutes(int8_t m)
	{
		data.Minutes += m + 60;
		rightTime();
	}
	void addHours(int8_t h)
	{
		data.Hours += h;
		rightTime();
	}
	RTC_TimeTypeDef* GetTime()
	{
		return &data;
	}

};
Time onTime, offTime(12,0), curTime;  // три объекта класса Time хранят заданное время включения и выключения и постоянно обновляемое текущее время.
Time* chsnTime = &curTime;    // указатель используется для выбор с каким конкретно временем будут работать все функции программы.
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_RTC_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
/* USER CODE BEGIN PFP */
void DigSet(uint8_t dig, uint8_t pos);
uint8_t DigitOut(uint8_t dig);
void SetOut(uint8_t dig);
uint8_t isBtn(uint16_t GPIO_Pin);
void showTime(uint8_t h, uint8_t m);
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
  MX_RTC_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
  EXTI->IMR &= ~EXTI_IMR_MR11;   // для установления текущего времени при подаче питания на микроконтроллер отключается прерывание на первой кнопке и запускается цикл
  while(what_dig_flag != NO_DIG) // ожидания изменения состояния флага what_dig_time на NO_DIG, что символизирует о настройке полного времени.
  {
	  showTime(chsnTime->GetHours(),chsnTime->GetMinutes());
  }
  if (HAL_RTC_SetTime(&hrtc, chsnTime->GetTime(), RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  EXTI->IMR |= EXTI_IMR_MR11;
  HAL_TIM_Base_Start_IT(&htim4);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  if(what_time_flag != NO_DISPLAY)              //с использованием указателя chsnTime, который взависимости от предыдущих действий пользователя может указывать на
	  {												//время включения, время выключения или текущее время.
		  if(what_time_flag == CURRENT_TIME)
			  HAL_RTC_GetTime(&hrtc, chsnTime->GetTime(), RTC_FORMAT_BIN);
	  	  showTime(chsnTime->GetHours(),chsnTime->GetMinutes());
	  }
	  else
	  {
		  HAL_RTC_GetTime(&hrtc, chsnTime->GetTime(), RTC_FORMAT_BIN);
		  HAL_Delay(500);
	  }
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL8;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef DateToUpdate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_ALARM;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0;
  sTime.Minutes = 0;
  sTime.Seconds = 0;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
  DateToUpdate.Month = RTC_MONTH_JANUARY;
  DateToUpdate.Date = 1;
  DateToUpdate.Year = 0;

  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 63999;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 50;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OnePulse_Init(&htim3, TIM_OPMODE_SINGLE) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 63999;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 40000;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, Lamp1_Pin|E_Pin|F_Pin|DP_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, G_Pin|GPIO_PIN_10|A_Pin|B_Pin
                          |C_Pin|D_Pin|D1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, D2_Pin|D3_Pin|D4_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA2 PA3 PA6 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : Lamp1_Pin E_Pin F_Pin DP_Pin */
  GPIO_InitStruct.Pin = Lamp1_Pin|E_Pin|F_Pin|DP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : G_Pin PB10 A_Pin B_Pin
                           C_Pin D_Pin D2_Pin D3_Pin
                           D4_Pin D1_Pin */
  GPIO_InitStruct.Pin = G_Pin|GPIO_PIN_10|A_Pin|B_Pin
                          |C_Pin|D_Pin|D2_Pin|D3_Pin
                          |D4_Pin|D1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI2_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

}

/* USER CODE BEGIN 4 */
//Вспомогательные функции DigOut, SetOut, DigSet необходимы для вывода данных на семисегментный индикатор
uint8_t DigOut(uint8_t dig)
{
	switch(dig)
	{
	case(0):
			return 0x7E;
			break;
	case(1):
		return 0x30;
			break;
	case(2):
		return 0x6D;
			break;
	case(3):
		return 0x79;
			break;
	case(4):
		return 0x33;
			break;
	case(5):
		return 0x5B;
			break;
	case(6):
		return 0x5F;
			break;
	case(7):
		return 0x70;
			break;
	case(8):
		return 0x7F;
			break;
	case(9):
		return 0x7B;
			break;
	}
	return 0;
}
void SetOut(uint8_t d)
{
	GPIOB->BRR = (1<<12 | 1 << 13 | 1<< 14 | 1 << 15 | 1 << 1);
	GPIOA->BRR = (1<<8 | 1 << 9  | 1<<11);
	GPIOB->BSRR = ((d & 0x40)<<6 | (d & 0x20)<<8 | (d & 0x10)<<10 | (d & 0x8)<<12 | (d & 0x1)<<1);
	GPIOA->BSRR = ((d & 0x4)<<6 | (d & 0x2)<<8);
}
void DigSet(uint8_t dig, uint8_t pos)
{
	HAL_GPIO_WritePin(GPIOB, D3_Pin|D4_Pin|D2_Pin|D1_Pin, GPIO_PIN_SET);
	SetOut(DigOut(dig));
	switch(pos)
	{
	case(1):
				HAL_GPIO_WritePin(GPIOB, D1_Pin, GPIO_PIN_RESET);
				break;
	case(2):
				HAL_GPIO_WritePin(GPIOA, DP_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOB, D2_Pin, GPIO_PIN_RESET);
				break;
	case(3):
				HAL_GPIO_WritePin(GPIOB, D3_Pin, GPIO_PIN_RESET);
				break;
	case(4):
				HAL_GPIO_WritePin(GPIOB, D4_Pin, GPIO_PIN_RESET);
				break;
	}
}

//Функция showTime выводит переданные в нее часы и минуты на семисегментный индикатор. В основном цикле программы она вызывается с различными временами,
//хранящимися в объектах curTime, onTime, offTime

void showTime(uint8_t h, uint8_t m)
{
	  DigSet(h/10,1);
	  HAL_Delay(2);
	  DigSet(h%10,2);
	  HAL_Delay(2);
	  DigSet(m/10,3);
	  HAL_Delay(2);
	  DigSet(m%10,4);
	  HAL_Delay(2);
	  HAL_GPIO_WritePin(GPIOB, D3_Pin|D4_Pin|D2_Pin|D1_Pin, GPIO_PIN_SET);
}

//Функция isBtn исползуется вместе с таймером №3 для борьбы с дребезгом контактов в обработчике внешних прерываний. Таймер 3 настроен на задержку в 50 мс.
//Его запуск происходит в обработчике прерывания, после срабатывания таймер выключается и вызывается callback функция внешнего прерывания.

uint8_t isBtn(uint16_t GPIO_Pin)
{
	if(HAL_GPIO_ReadPin(GPIOA,GPIO_Pin) == GPIO_PIN_SET)
		return 1;
	else
		return 0;
}

//callback функция внешних прерывания обрабатывает 4 кнопки. Действие кнопок зависит от двух глобальных флагов what_time_flag и what_dig_flag
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
		if(GPIO_Pin == GPIO_PIN_0)  // кнопка один отвечает за выбор времени на дисплее. Возможно 4 состояния: отсутсвие индикации, текущее время, время включение лампы, время выключения лампы.
		{
			if(isBtn(GPIO_Pin))
			{
				switch(++what_time_flag)
				{
					case CURRENT_TIME:
					{
						HAL_TIM_Base_Stop_IT(&htim4);
						HAL_TIM_Base_Start_IT(&htim4);
						break;
					}
					case ON_TIME:
					{
						chsnTime = &onTime;  // указатель на объект класса Time используется для выбора с каким временем будет произведены действия
						what_dig_flag = HOURS_DIG;
						break;
					}
					case OFF_TIME:
					{
						chsnTime = &offTime;
						what_dig_flag = HOURS_DIG;
						break;
					}
					case 4:
					{
						chsnTime = &curTime;
						what_time_flag = CURRENT_TIME;
						TIM4->CNT = 0;
						what_dig_flag = NO_DIG;
						break;
					}
				}

			}
		}
		else if(GPIO_Pin == GPIO_PIN_2)  // кнопки 2 и 3 активны при выборе времени включения и выключения лампы. Кнопка 2 увеличивает выбранную цифру, кнопка 3 - уменьшает.
		{
			if(isBtn(GPIO_Pin) && what_time_flag !=NO_DISPLAY && what_time_flag != CURRENT_TIME)
				switch(what_dig_flag)
				{
				case(HOURS_DIG):
					{
						chsnTime->addHours(1);
						break;
					}
				case(TEN_MINUTES_DIG):
					{
						chsnTime->addMinutes(10);
						break;
					}
				case(MINUTES_DIG):
					{
						chsnTime->addMinutes(1);
						break;
					}
				}
		}
		else if(GPIO_Pin == GPIO_PIN_3)
		{
			if(isBtn(GPIO_Pin) && what_time_flag != NO_DISPLAY && what_time_flag != CURRENT_TIME)
				switch(what_dig_flag)
				{
				case(HOURS_DIG):
					{
						chsnTime->addHours(-1);
						break;
					}
				case(TEN_MINUTES_DIG):
					{
						chsnTime->addMinutes(-10);
						break;
					}
				case(MINUTES_DIG):
					{
						chsnTime->addMinutes(-1);
						break;
					}
				}
		}
		else if(GPIO_Pin == GPIO_PIN_6) //Кнопка 4 активна при выборе времени включения и выключения. Отвечает за цифру, которая изменяется кнопками 2 и 3.
										//Это часы, минуты, десятки минут и единицы минут.
		{
			if(isBtn(GPIO_Pin) && what_time_flag != NO_DISPLAY && what_time_flag != CURRENT_TIME)
				if(++what_dig_flag == 4)
				{
					what_dig_flag = NO_DIG;
					what_time_flag = CURRENT_TIME;
					chsnTime = &curTime;
				}
		}
		else
			HAL_Delay(1);
}

//callback функция таймеров 3 и 4. При прерывании 3 таймером он выключается, возникашая задержка использует для борьбы с дребезгом контактов. Состояние таймера №3 отслеживается
//в обработчике прерывания кнопок.
//Таймер №4 с периодом в 45 секунд используется для 1)выключения дисплея и 2)проверки необходимо ли включить или выключить лампу.

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM3)
		HAL_TIM_Base_Stop_IT(htim);
	else if(htim->Instance == TIM4)
	{
		if(what_time_flag == CURRENT_TIME)
		{
			HAL_GPIO_WritePin(GPIOB, D4_Pin, GPIO_PIN_SET);
			what_time_flag = NO_DISPLAY;
		}
		if(curTime.GetHours() == onTime.GetHours() && curTime.GetMinutes() == onTime.GetMinutes())
			HAL_GPIO_WritePin(GPIOA, Lamp1_Pin, GPIO_PIN_SET);
		else if(curTime.GetHours() == offTime.GetHours() && curTime.GetMinutes() == offTime.GetMinutes())
			HAL_GPIO_WritePin(GPIOA,Lamp1_Pin,GPIO_PIN_RESET);
	}
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
