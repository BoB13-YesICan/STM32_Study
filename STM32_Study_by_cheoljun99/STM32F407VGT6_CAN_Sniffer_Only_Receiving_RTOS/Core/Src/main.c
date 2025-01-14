/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
#include "stdbool.h"
#include <inttypes.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef struct qCANMsg {
	uint8_t data[8];
	CAN_RxHeaderTypeDef header;
} CANMessage;

typedef enum {
	SNIFFER_STOPPED = 0x00, SNIFFER_ACTIVE = 0x01,
} SnifferAtivityStatus;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define osAnySignal 0x00
#define SIZE_RX 20
#define noWait 0
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan1;

TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart1;

osThreadId defaultTaskHandle;
/* USER CODE BEGIN PV */

osThreadId forwardDatagramTaskId;
osThreadId receivedDatagramTaskId;

osThreadId idleTaskId;

osMailQDef(canDatagramsQueue, 100, CANMessage);		// Define mail queue
osMailQId canDatagramsQueue;

// Peripherals ISR global variables

CAN_RxHeaderTypeDef rxMessageHeader;
uint8_t rxDataReceived[8];


// Sneffer activity status variable
SnifferAtivityStatus snifferAtivityStatus = SNIFFER_ACTIVE;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM1_Init(void);
void StartDefaultTask(void const * argument);

/* USER CODE BEGIN PFP */
void bootLEDIndicator(void);

void fordwardDatagramsThread(void const*);
void recieivedDatagramsThread(void const*);
void idleThread(void const*);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define NUMBER_BLINKS 20

void bootLEDIndicator(void) {
	int blinkCounter = 0;
	while (blinkCounter < NUMBER_BLINKS) {
		HAL_GPIO_TogglePin(ORANGE_LED_SYSTEM_GPIO_Port, ORANGE_LED_SYSTEM_Pin);
		HAL_Delay(30);
		blinkCounter++;
	}
	HAL_GPIO_WritePin(ORANGE_LED_SYSTEM_GPIO_Port, ORANGE_LED_SYSTEM_Pin, GPIO_PIN_SET);
}


void setSinfferCANFilter(void) {
	/* Default filter - accept all to CAN_FIFO*/
	CAN_FilterTypeDef sFilterConfig;
	sFilterConfig.FilterBank = 0;
	sFilterConfig.FilterIdHigh = 0x00005;
	sFilterConfig.FilterBank = 0x0000;
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	sFilterConfig.FilterIdHigh = 0x200 << 5;  //11-bit ID, in top bits
	sFilterConfig.FilterIdLow = 0x0000;
	sFilterConfig.FilterMaskIdHigh = 0x0000;
	sFilterConfig.FilterMaskIdLow = 0x0000;
	sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	sFilterConfig.FilterActivation = ENABLE;

	HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig);
}


uint8_t serializeDatagram(uint8_t *pExitBuffer, CAN_RxHeaderTypeDef receivedCANHeader, uint8_t *rxData) {
    uint8_t cursor = 0;

<<<<<<< HEAD
    // TIM1?�� 마이?��로초 값을 먼�? ?��?��?��
    uint32_t microseconds = __HAL_TIM_GET_COUNTER(&htim1);  // TIM1 ???���? 카운?�� �? (마이?��로초 ?��?��)

    // �? ?�� HAL_GetTick()?�� ?��?��?�� �?리초 값을 ?��?��?��
    uint32_t millis = HAL_GetTick();

    // �?리초 값을 ?��/�?/초로 �??��
    uint32_t ms = millis % 1000;           // �?리초 (0-999)
    uint32_t seconds = (millis / 1000) % 60;   // �? (0-59)
    uint32_t minutes = (millis / 60000) % 60;  // �? (0-59)
    uint32_t hours = (millis / 3600000) % 24;  // ?���? (0-23)

    // HH:MM:SS.mmm.uuu ?��?��?���? ???��?��?��?�� 출력
    cursor += sprintf((char*) pExitBuffer + cursor, "[%02lu:%02lu:%02lu.%03lu.%03lu] ",
                      hours, minutes, seconds, ms, microseconds);

    // CAN ?��별자 출력 (?��?�� ID ?��?�� ?���? ID)
=======
    // TIM1?�� 마이?��로초 값을 먼�? ?��?��?��
    uint32_t microseconds = __HAL_TIM_GET_COUNTER(&htim1);  // TIM1 ???���? 카운?�� �? (마이?��로초 ?��?��)

    // �? ?�� HAL_GetTick()?�� ?��?��?�� �?리초 값을 ?��?��?��
    uint32_t millis = HAL_GetTick();

    // �?리초 값을 ?��/�?/초로 �??��
    uint32_t ms = millis % 1000;           // �?리초 (0-999)
    uint32_t seconds = (millis / 1000) % 60;   // �? (0-59)
    uint32_t minutes = (millis / 60000) % 60;  // �? (0-59)
    uint32_t hours = (millis / 3600000) % 24;  // ?���? (0-23)

    // HH:MM:SS.mmm.uuu ?��?��?���? ???��?��?��?�� 출력
    cursor += sprintf((char*) pExitBuffer + cursor, "[%02lu:%02lu:%02lu.%03lu.%03lu] ",
                      hours, minutes, seconds, ms, microseconds);

    // CAN ?��별자 출력 (?��?�� ID ?��?�� ?���? ID)
>>>>>>> d11152295f41fdfa255a00a18ae3432128dc5ba6
    if (receivedCANHeader.IDE == CAN_ID_EXT) {
        cursor += sprintf((char*) pExitBuffer + cursor, "EXT CAN ID: 0x%" PRIx32 " ", receivedCANHeader.ExtId);
    } else {
        cursor += sprintf((char*) pExitBuffer + cursor, "STD CAN ID: 0x%" PRIx32 " ", receivedCANHeader.StdId);
    }

    // DLC (Data Length Code) 출력
    cursor += sprintf((char*) pExitBuffer + cursor, "DLC: %lu ", (unsigned long)receivedCANHeader.DLC);

<<<<<<< HEAD
    // ?��?��?�� 출력 (?��?��?�� ?��?��?��?�� 경우)
=======
    // ?��?��?�� 출력 (?��?��?�� ?��?��?��?�� 경우)
>>>>>>> d11152295f41fdfa255a00a18ae3432128dc5ba6
    if (receivedCANHeader.RTR == CAN_RTR_DATA) {
        cursor += sprintf((char*) pExitBuffer + cursor, "Data: ");
        for (int i = 0; i < receivedCANHeader.DLC; i++) {
            cursor += sprintf((char*) pExitBuffer + cursor, "%02X ", rxData[i]);
        }
    }

<<<<<<< HEAD
    // ?��?�� 개행 추�?
=======
    // ?��?�� 개행 추�?
>>>>>>> d11152295f41fdfa255a00a18ae3432128dc5ba6
    cursor += sprintf((char*) pExitBuffer + cursor, "\r\n");

    return cursor;
}



 void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
  	if (HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &rxMessageHeader,rxDataReceived) == HAL_OK) {
  		if (snifferAtivityStatus == SNIFFER_ACTIVE) {
<<<<<<< HEAD
  			 // ?��?��?�� CAN ?��?��?���? UART�? 출력 (�??��?�� 최소 ?��?���? ISR?��?�� ?��?��)
=======
  			 // ?��?��?�� CAN ?��?��?���? UART�? 출력 (�??��?�� 최소 ?��?���? ISR?��?�� ?��?��)
>>>>>>> d11152295f41fdfa255a00a18ae3432128dc5ba6
  			osSignalSet(receivedDatagramTaskId, osAnySignal);

  		}
  	}
  	HAL_GPIO_TogglePin(RED_LED_CANRX_GPIO_Port,RED_LED_CANRX_Pin);

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
  MX_CAN1_Init();
  MX_USART1_UART_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
  setSinfferCANFilter();
  HAL_CAN_Start(&hcan1);
  HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);

  // Start TIM1 for microsecond timestamping
  HAL_TIM_Base_Start(&htim1);


	/* Code to make a state sniffer state without using a dedicated GUI app*/
//	HAL_CAN_Start(&hcan1);
//	if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) == HAL_OK) {
//		HAL_GPIO_TogglePin(GREEN_LED_GPIO_Port,GREEN_LED_Pin);
//	}

  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  canDatagramsQueue = osMailCreate(osMailQ(canDatagramsQueue), NULL);
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */

  osThreadDef(forwardDatagramsTask, fordwardDatagramsThread, osPriorityNormal,0, 640);
  forwardDatagramTaskId = osThreadCreate(osThread(forwardDatagramsTask),NULL);

  osThreadDef(receiveDatagramsTask, recieivedDatagramsThread,osPriorityNormal, 0, 640);
  receivedDatagramTaskId = osThreadCreate(osThread(receiveDatagramsTask),NULL);

  osThreadDef(idleTask, idleThread, osPriorityIdle, 0, 128);
  idleTaskId = osThreadCreate(osThread(idleTask), NULL);

  bootLEDIndicator();

  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
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
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 3;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_11TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */

  /* USER CODE END CAN1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 83;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 1000;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GREEN_LED_IDLE_Pin|ORANGE_LED_SYSTEM_Pin|RED_LED_CANRX_Pin|BLUE_LED_CANTX_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : GREEN_LED_IDLE_Pin ORANGE_LED_SYSTEM_Pin RED_LED_CANRX_Pin BLUE_LED_CANTX_Pin */
  GPIO_InitStruct.Pin = GREEN_LED_IDLE_Pin|ORANGE_LED_SYSTEM_Pin|RED_LED_CANRX_Pin|BLUE_LED_CANTX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void recieivedDatagramsThread(void const *argument) {
	//CAN_RxHeaderTypeDef rxMessageHeader;
	//uint8_t rxDataReceived[8];
	osEvent evt;
	while (true) {
		evt = osSignalWait(osAnySignal, 1);//osWaitForever
		if (evt.status == osEventSignal) {
			CANMessage *msgToSend = osMailCAlloc(canDatagramsQueue, noWait);
			if (msgToSend != NULL) {
				 msgToSend->header = rxMessageHeader;
				 memcpy(msgToSend->data, rxDataReceived, sizeof(rxDataReceived));
				 osMailPut(canDatagramsQueue, msgToSend);
			}
			osThreadYield();
		}
		else{
			osThreadYield();
		}

	}
}

/*
void fordwardDatagramsThread(void const *argument) {
	uint8_t serializedDatagram[24] = { 0 };
	uint8_t lenSerialized;
	uint8_t encodedDatagram[24] = { 0 };
	osEvent evt;
	CANMessage *dequeuedMsg;
	while (true) {
		evt = osMailGet(canDatagramsQueue, 1);	//osWaitForever
		if (evt.status == osEventMail) {
			dequeuedMsg = evt.value.p;
			lenSerialized = serializeDatagram(serializedDatagram,dequeuedMsg->header, dequeuedMsg->data);
			cobs_encode_result result = cobs_encode(encodedDatagram, 24,serializedDatagram, lenSerialized + 1);
			if (result.status == COBS_ENCODE_OK) {
				// Append Zero byte to delimiter frame boundary
				encodedDatagram[result.out_len + 1] = 0x00;
				HAL_UART_Transmit(&huart1, encodedDatagram, result.out_len + 1,50);
				// Toggle monitoring line to indicate a successful datagrams retransmission
				memset(encodedDatagram, 0, sizeof(encodedDatagram));
			}
			osMailFree(canDatagramsQueue, dequeuedMsg);
			osThreadYield();

		}else{
			osThreadYield();
		}
	}
}
*/

void fordwardDatagramsThread(void const *argument) {
	uint8_t serializedDatagram[100] = { 0 };
	uint8_t lenSerialized;
	osEvent evt;
	CANMessage *dequeuedMsg;
	while (true) {
		evt = osMailGet(canDatagramsQueue, 1);	//osWaitForever
		if (evt.status == osEventMail) {
			dequeuedMsg = evt.value.p;
			lenSerialized = serializeDatagram(serializedDatagram,dequeuedMsg->header, dequeuedMsg->data);
			HAL_UART_Transmit(&huart1, serializedDatagram, lenSerialized,HAL_MAX_DELAY);
			HAL_GPIO_TogglePin(BLUE_LED_CANTX_GPIO_Port, BLUE_LED_CANTX_Pin);
			osMailFree(canDatagramsQueue, dequeuedMsg);
			osThreadYield();

		}else{
			osThreadYield();
		}
	}
}


void idleThread(void const *argument) {
	while (true) {
		HAL_GPIO_TogglePin(GREEN_LED_IDLE_GPIO_Port,GREEN_LED_IDLE_Pin);
		osDelay(100);
	}
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END 5 */
}

 /**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
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
