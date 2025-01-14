/* USER CODE BEGIN Header */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "stdio.h"
#include "string.h"
#include "cobs.h"
#include "cQueue.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

uint16_t num=250;
char data[300]="";
char str[]="hello world!\n";


#define SIZE_RX 30

typedef struct qCmd {
	uint8_t commnddBuff[SIZE_RX];
	uint8_t commandSize;
} EnueuedCommand;

typedef struct qCANMsg {
	uint8_t data[8];
	CAN_RxHeaderTypeDef header;
} EncuedCANMsg;

typedef enum {
	SNIFFER_STOPPED = 0x00, SNIFFER_ACTIVE = 0x01,
} SnifferAtivityStatus;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define	IMPLEMENTATION	FIFO
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan1;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */

// CAN Callback variables
CAN_RxHeaderTypeDef rxMessageHeader;
uint8_t rxDataReceived[8];

// UART Callback variables
uint8_t rxUARTBuff[1] = { 0 };
uint8_t rxCommandBuff[SIZE_RX] = { 0 };
volatile uint8_t cursor = 0;
uint8_t decodedCommand[SIZE_RX] = { 0 };

#define CMD_QUEUE_SIZE 100
Queue_t commandQueue;
EnueuedCommand decuedComand;

#define CAN_MSSG_QUEUE_SIZE 100
Queue_t canMsgQueue;
EncuedCANMsg decuedCANMssg;

SnifferAtivityStatus snifferAtivityStatus = SNIFFER_STOPPED;

volatile uint32_t miliseconds = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN1_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */

int __io_putchar(int);
void blueLEDIndicator(void);
void blinkLed();
void processCANMsg(void);
void processComand(void);
void processBitRateCommand();
void processMessageComand(void);
void processLoopBackModeCommand(void);
void processRebootCommand(void);
void processActivitySniferComand();
void sendCANMDummyessage(void);
uint32_t sendCANMessage(uint8_t, uint32_t, bool, bool, uint8_t*);
void setCANFilterAcceptAll(void);
void setCANFilterAcceptSingleExtendedID(uint32_t, uint32_t);
void setCANFilterAcceptSingleStandardID(uint32_t,uint16_t);
void setSinfferCANFilterAcceptRange(uint8_t, uint32_t);
void setDatagramTypeIdentifer(uint32_t, uint32_t, uint8_t*, uint8_t*);
void setFormatedDatagramIdentifer(uint32_t, uint8_t*, uint8_t*, int);
void setDatagramIdentifer(CAN_RxHeaderTypeDef, uint8_t*, uint8_t*);
void setDLC(uint32_t, uint8_t*, uint8_t*);
void setData(uint8_t*, int, uint8_t*, uint8_t*);
uint8_t serializeDatagram(uint8_t*, CAN_RxHeaderTypeDef, uint8_t*);
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef*);
void processCANMsg(void);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef*);
void substr(char *str, char*, int, int);
int toInteger(uint8_t *stringToConvert, int len);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void generateDummyCANTraffic(uint32_t);
void sendCANStdDataDummyMessage(void);
void sendCANExtDataDummyMessage(void);
void sendCANStdRemoteDummyMessage(void);
void sendCANExtRemoteDummyMessage(void);
void ErrorAppHandler(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

int __io_putchar(int ch) {
	HAL_UART_Transmit(&huart3, (uint8_t*) &ch, 1, 100);
	return ch;
}
#define NUMBER_BLINKS 20
void blueLEDIndicator(void) {
	int blinkCounter = 0;
	while (blinkCounter < NUMBER_BLINKS) {
		HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin);
		HAL_GPIO_TogglePin(PIN2_GPIO_Port, PIN2_Pin);
		HAL_Delay(30);
		blinkCounter++;
	}
	HAL_GPIO_WritePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(PIN2_GPIO_Port, PIN2_Pin, GPIO_PIN_SET);
}

void ErrorAppHandler(void) {
	// Display something went wrong :(
	 blueLEDIndicator();
}
/**
 * @brief  Sends a dummy message over the CAN bus using the hcan1 handle
 * @param  none
 * @retval None
 */
void sendCANDummyMessage(void) {
	uint32_t TxMailbox;
	CAN_TxHeaderTypeDef pHeader;
	pHeader.DLC = 8; //give message size of 1 byte
	pHeader.IDE = CAN_ID_STD; //set identifier to standard
	pHeader.RTR = CAN_RTR_DATA; //set data type to remote transmission request?CAN_RTR_DATA
	pHeader.StdId = 0x200; //define a standard identifier, used for message identification by filters (switch this for the other microcontroller)
	//uint8_t messageLoadBuffer[8] = {'B','L','A','C','K','_','&','#'};
	uint8_t messageLoadBuffer[8] = { 42, 0x4c, 0x41, 0x43, 0x4b, 0x5f, 0x26,
			0x23 };
	if (HAL_CAN_AddTxMessage(&hcan1, &pHeader, messageLoadBuffer, &TxMailbox)
			!= HAL_OK) {
		ErrorAppHandler();
	}
}
/**
 * @brief  Sends a CAN message through the CAN controller
 * @param  dlc: data length code (number of bytes of data to be sent)
 * @param  msgID: ID of the message to be sent
 * @param  isRTR: flag indicating if the message is a Remote Transmission Request (RTR)
 * @param  isStandard: flag indicating if the message ID is standard (11-bit) or extended (29-bit)
 * @param  data: pointer to the data to be sent
 * @retval TxMailbox: mailbox used for transmission
 */

uint32_t sendCANMessage(uint8_t dlc, uint32_t msgID, bool isRTR,
		bool isStandard, uint8_t *data) {

	uint32_t TxMailbox;
	CAN_TxHeaderTypeDef pHeader;
	pHeader.DLC = dlc;

	if (isStandard) {
		pHeader.IDE = CAN_ID_STD;
		pHeader.StdId = msgID;
	} else {
		pHeader.IDE = CAN_ID_EXT;
		pHeader.ExtId = msgID;
	}
	if (isRTR) {
		pHeader.RTR = CAN_RTR_REMOTE;
	} else {
		pHeader.RTR = CAN_RTR_DATA;
	}

	// CAN λ©μμ§? ? ?‘
	if (HAL_CAN_AddTxMessage(&hcan1, &pHeader, data, &TxMailbox) == HAL_OK) {
		// λ©μμ§? ? ?‘ ?±κ³? ? ORANGE_LED μΌκΈ°
		HAL_GPIO_TogglePin(ORANGE_LED_GPIO_Port, ORANGE_LED_Pin);
	} else {
	    // λ©μμ§? ? ?‘ ?€?¨ ? ??¬ μ²λ¦¬
	    ErrorAppHandler();
	}

	return TxMailbox;
}
/**
 * @brief  Configures the CAN filter to accept all messages to the CAN_FIFO buffer
 * @param  None
 * @retval None
 */
void setCANFilterAcceptAll(void) {
	/* Default filter - accept all to CAN_FIFO*/
	CAN_FilterTypeDef sFilterConfig;
	sFilterConfig.FilterBank = 0;
	//sFilterConfig.FilterIdHigh = 0x00005;
	sFilterConfig.FilterBank = 0x0000;
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	// Set Mask to all 0s to allow pass any messageID
	sFilterConfig.FilterMaskIdHigh = 0x0000;
	sFilterConfig.FilterMaskIdLow = 0x0000;
	sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	sFilterConfig.FilterActivation = ENABLE;
	if (HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK) {
		ErrorAppHandler();
	}
}


void setCANFilterAcceptSingleStandardID(uint32_t filterBank, uint16_t acceptedID){
	/* Default filter - accept all to CAN_FIFO*/
	CAN_FilterTypeDef sFilterConfig;
	sFilterConfig.FilterBank = 0;
	sFilterConfig.FilterBank = filterBank;
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	sFilterConfig.FilterIdHigh = acceptedID << 5; //11-bit ID, in top bits 6= 16-11 bits
	sFilterConfig.FilterIdLow = 0x0000;
	sFilterConfig.FilterMaskIdHigh = 0x7FF << 5;
	sFilterConfig.FilterMaskIdLow = 0x0000;
	sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	sFilterConfig.FilterActivation = ENABLE;
	if (HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK) {
		ErrorAppHandler();
	}
}




void setCANFilterAcceptSingleExtendedID(uint32_t filterBank, uint32_t acceptedID){
	uint16_t low16bits = acceptedID & 0xFFFF;
	uint16_t high16Bits = (acceptedID >> 16) & 0xFFFF;
    /* Default filter - accept all to CAN_FIFO*/
	CAN_FilterTypeDef sFilterConfig;
	sFilterConfig.FilterBank = 0;
	sFilterConfig.FilterBank = filterBank;
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	sFilterConfig.FilterIdHigh = high16Bits;
	sFilterConfig.FilterIdLow = low16bits << 3; //11-bit ID, in top bits 3=(32-29)
	sFilterConfig.FilterMaskIdHigh = 0xFFFF;
	sFilterConfig.FilterMaskIdLow = 0xFFF8;
	sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	sFilterConfig.FilterActivation = ENABLE;
	if (HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK) {
		ErrorAppHandler();
	}
}

/**
 * setDatagramTypeIdentifier
 *
 * @brief sets the type identifier of a CAN (Controller Area Network) datagram
 *
 * This function takes in four parameters:
 * - ide: an unsigned 32-bit integer representing the ID format of the datagram (either CAN_ID_STD for standard ID format or CAN_ID_EXT for extended ID format)
 * - rtr: an unsigned 32-bit integer representing the type of the datagram (either CAN_RTR_DATA for data frame or CAN_RTR_REMOTE for remote frame)
 * - pExitBuffer: a pointer to the buffer where the datagram will be stored
 * - pCursor: a pointer to an integer representing the current position of the cursor in the buffer
 *
 * The function first checks the value of ide to determine whether the datagram uses a standard or extended ID format.
 * If ide is CAN_ID_STD, the function checks the value of rtr to determine whether the datagram is a data or remote frame.
 * If rtr is CAN_RTR_DATA, the function sets the first byte of the datagram to 't' and if rtr is CAN_RTR_REMOTE,
 * the function sets the first byte of the datagram to 'r'.
 *
 * If ide is CAN_ID_EXT, the function again checks the value of rtr to determine whether the datagram is a data or remote frame.
 * If rtr is CAN_RTR_DATA, the function sets the first byte of the datagram to 'T' and if rtr is CAN_RTR_REMOTE,
 * the function sets the first byte of the datagram to 'R'.
 *
 * Finally, the function increments the cursor position by 1.
 *
 * @param ide - unsigned 32-bit int, representing the ID format of the datagram
 * @param rtr - unsigned 32-bit int, representing the type of the datagram
 * @param pExitBuffer - pointer to the buffer where the datagram will be stored
 * @param pCursor - pointer to an integer representing the current position of the cursor in the buffer
 *
 * @return void
 */
void setDatagramTypeIdentifer(uint32_t ide, uint32_t rtr, uint8_t *pExitBuffer,
		uint8_t *pCursor) {
	if (ide == CAN_ID_STD) {
		if (rtr == CAN_RTR_DATA) {
			pExitBuffer[*pCursor] = 't';
		} else if (rtr == CAN_RTR_REMOTE) {
			pExitBuffer[*pCursor] = 'r';
		}
	} else if (ide == CAN_ID_EXT) {

		if (rtr == CAN_RTR_DATA) {
			pExitBuffer[*pCursor] = 'T';
		} else if (rtr == CAN_RTR_REMOTE) {
			pExitBuffer[*pCursor] = 'R';
		}
	}
	*pCursor = *pCursor + 1;
}
/**
* @brief  Formats a given ID number and copies it to a buffer at a specific cursor position
* @param  idNum: the ID number to be formatted and copied
* @param  pExitBuffer: pointer to the buffer where the formatted ID will be copied
* @param  pCursor: pointer to the cursor position in the buffer
* @param  len: the desired length of the formatted ID
* @retval None
*/
void setFormatedDatagramIdentifer(uint32_t idNum, uint8_t *pExitBuffer,uint8_t *pCursor, int len) {

	char *id = (char*) malloc(sizeof(char) * (len + 1));
	int numOfDigits = 0;
	int valueToConsume = idNum;

	while (valueToConsume != 0) {
		valueToConsume /= 10;     // n = n/10
		++numOfDigits;
	}

	sprintf(id + (len - numOfDigits), "%d", (int) idNum);
	for (int eraser = 0; eraser < (len - numOfDigits); eraser++) {
		id[eraser] = '0';
	}
	memcpy((char*) pExitBuffer + *pCursor, id, strlen(id)+1);
	free(id);
	*pCursor = *pCursor + len;
}
/**
* @brief  Extracts the ID of a received CAN message and formats it to a desired length, then copies it to
*         a buffer at a specific cursor position
* @param  receivedCANHeader: the header of the received CAN message, containing the ID of the message
* @param  pExitBuffer: pointer to the buffer where the formatted ID will be copied
* @param  pCursor: pointer to the cursor position in the buffer
* @retval None
*/
void setDatagramIdentifer(CAN_RxHeaderTypeDef receivedCANHeader,
		uint8_t *pExitBuffer, uint8_t *pCursor) {
	if (receivedCANHeader.IDE == CAN_ID_EXT) {
		setFormatedDatagramIdentifer(receivedCANHeader.ExtId, pExitBuffer,
				pCursor, 9);
	}
	if (receivedCANHeader.IDE == CAN_ID_STD) {
		setFormatedDatagramIdentifer(receivedCANHeader.StdId, pExitBuffer,
				pCursor, 4);

	}
}
/**
 * @brief  Formats and copies the data length code (DLC) of a received CAN message to a buffer at a specific cursor position
 * @param  dlc: the data length code of the received message
 * @param  pExitBuffer: pointer to the buffer where the DLC will be copied
 * @param  pCursor: pointer to the cursor position in the buffer
 * @retval None
 */
void setDLC(uint32_t dlc, uint8_t *pExitBuffer, uint8_t *pCursor) {
    sprintf((char*) pExitBuffer + *pCursor, "%d", (int) dlc);
	*pCursor = *pCursor + 1;
}
/**
 * @brief  Copy data from source to destination buffer at specific cursor position
 * @param  data: pointer to the source data
 * @param  dlc: data length code
 * @param  pExitBuffer: pointer to the destination buffer
 * @param  pCursor: pointer to the cursor position in the destination buffer
 * @retval None
 */
void setData(uint8_t *data, int dlc, uint8_t *pExitBuffer, uint8_t *pCursor) {
	for (int counter = 0; counter < dlc; counter++) {
		pExitBuffer[*pCursor + counter] = data[counter];
	}
	*pCursor = *pCursor + dlc;
}
/**
 * @brief Serialize a CAN datagram and store it in a buffer
 * @param pExitBuffer: pointer to the buffer where the serialized datagram will be stored
 * @param receivedCANHeader: struct containing the header information of the received CAN message
 * @param rxData: pointer to the received data payload
 * @retval cursor: the final position of the cursor in the buffer
 */
uint8_t serializeDatagram(uint8_t *pExitBuffer,
		CAN_RxHeaderTypeDef receivedCANHeader, uint8_t *rxData) {

	uint8_t cursor = 0;
	setDatagramTypeIdentifer(receivedCANHeader.IDE, receivedCANHeader.RTR,
			pExitBuffer, &cursor);
	setDatagramIdentifer(receivedCANHeader, pExitBuffer, &cursor);
	setDLC(receivedCANHeader.DLC, pExitBuffer, &cursor);
	if (receivedCANHeader.RTR == CAN_RTR_DATA) {
		setData(rxData, receivedCANHeader.DLC, pExitBuffer, &cursor);
	}

	return cursor;
}

#define ZERO_BYTE '0x00'

/**
 * @brief Interrupt callback function for CAN RX FIFO0 message pending. For each received Messsge a new
 *        EncuedCANMsg is created and stored in the Queue to be processed when corresponding
 * @param hcan1: pointer to the CAN handle
 * @retval None
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan1) {

	if (HAL_CAN_GetRxMessage(hcan1, CAN_RX_FIFO0, &rxMessageHeader,rxDataReceived) == HAL_OK) {
		HAL_GPIO_TogglePin(PIN3_GPIO_Port, PIN3_Pin);

		// STM32F407 λ³΄λ?? CAN λ©μμ§? ??  ? LED ? κΈ?
		HAL_GPIO_TogglePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin);

		/*?λ²κΉ μ½λ USART3? ?? ? CAN ?°?΄?°λ₯? μΆλ ₯?κΈ? ??΄? ?¬?©*/
//		// ?μ€?/??₯ CAN ID? ?°?Ό μΆλ ₯ λ©μμ§? ??±
//		if (rxMessageHeader.IDE == CAN_ID_STD) {
//		    sprintf(data, "Standard CAN ID: 0x%" PRIx32 "\r\n", rxMessageHeader.StdId);
//		} else {
//		    sprintf(data, "Extended CAN ID: 0x%" PRIx32 "\r\n", rxMessageHeader.ExtId);
//		}
//
//		// UARTλ₯? ?΅?΄ CAN ID ? ?‘ (?λ¬? λΉ λ₯Έ ? ?‘? ?Ό?κΈ? ??΄ ? ? ? ?¬κΈ?)
//		HAL_UART_Transmit(&huart3, (uint8_t*)data, strlen(data), HAL_MAX_DELAY);


		EncuedCANMsg msg;
		msg.header = rxMessageHeader;
		memcpy((char*) msg.data, (char*) rxDataReceived,rxMessageHeader.DLC);
		q_push(&canMsgQueue, &msg);
	} else {
		ErrorAppHandler();
	}

}
/**
 * @brief Process a received CAN message extracted from the Queue serializing it and then
 *        applying COBS encoding to sent it using UART
 * @retval None
 */
void processCANMsg() {

	uint8_t parsedDatagram[24] = { 0 };
	uint8_t lenParsed = serializeDatagram(parsedDatagram, decuedCANMssg.header,
			decuedCANMssg.data);

	uint8_t encodedDatagram[24] = { 0 };
	cobs_encode_result result = cobs_encode(encodedDatagram, 24, parsedDatagram,
			lenParsed + 1);

	if (result.status == COBS_ENCODE_OK) {
		// Append Zero byte to delimiter frame boundary
		encodedDatagram[result.out_len + 1] = 0x00;
		HAL_UART_Transmit(&huart3, encodedDatagram, result.out_len + 1, 50);

		// Toggle monitoring line to indicate a successful datagram retransmission
		HAL_GPIO_TogglePin(RED_LED_GPIO_Port,RED_LED_Pin);
	}
}

#define NUMBER_BOOT_BLINKS 5

void blinkLed() {
	int blinkCounter = 0;
	while (blinkCounter < NUMBER_BOOT_BLINKS) {
		HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin);
		HAL_Delay(50);
		blinkCounter++;
	}
	HAL_GPIO_WritePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin, GPIO_PIN_SET);
}
/**
 * @brief Interrupt callback function for UART RX Complete event. each time a new bute is recived is
 *        processed and stored inside rxCommandBuff until the last character is received and then the
 *        rxCommandBuff is used t create a new EnueuedCommand that is stored ant te Queue pending to
 *        be processed.
 * @param huart: pointer to the UART handle
 * @retval None
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == huart3.Instance) {
		HAL_GPIO_TogglePin(PIN1_GPIO_Port, PIN1_Pin);
		rxCommandBuff[cursor] = rxUARTBuff[0];
		cursor++;
		if (cursor >= SIZE_RX) {
			cursor = 0;
		}
		if (rxUARTBuff[0] == 0x00) {
			EnueuedCommand pendingCmd;
			memcpy((char*) pendingCmd.commnddBuff, (char*) rxCommandBuff, cursor);
			pendingCmd.commandSize = cursor - 1;
			if (!q_isFull(&commandQueue)) {
				q_push(&commandQueue, &pendingCmd);
			}
			cursor = 0;
			HAL_GPIO_TogglePin(PIN2_GPIO_Port, PIN2_Pin);
		}
		// Enable interrupt for again to be able to continue receiving data
		HAL_UART_Receive_IT(&huart3, rxUARTBuff, 1);
	}

}
/**
 * @brief Extracts a substring from a string
 * @param str: pointer to the original string
 * @param sub: pointer to the substring buffer
 * @param start: starting index of the substring
 * @param len: length of the substring
 * @retval None
 */
void substr(char *str, char *sub, int start, int len) {
	memcpy(sub, &str[start], len);
	sub[len] = '\0';
}
/**
 * @brief Converts a string of digits to an integer
 * @param stringToConvert: pointer to the string of digits
 * @param len: length of the string
 * @retval The integer value of the string
 */
int toInteger(uint8_t *stringToConvert, int len) {
	int counter = len - 1;
	int exp = 1;
	int value = 0;
	while (counter >= 0) {
		value = value + (stringToConvert[counter] - '0') * exp;
		exp = exp * 10;
		counter--;
	}
	return value;
}
/**
 * @brief Process each Message command received from  the UART interface and
 *        send the apropiate CAN Message.
 * @retval None
 * @retval None
 */
void processMessageComand() {
	uint8_t cursor = 1;

	if (decodedCommand[cursor] == 'T') {
		cursor++;
		uint8_t messageID_str[9];
		substr((char*) decodedCommand, (char*) messageID_str, cursor, 9);

		int msgID = toInteger(messageID_str, 9);
		cursor += 9;

		uint8_t dlc_str[1];
		substr((char*) decodedCommand, (char*) dlc_str, cursor, 1);
		uint8_t dlc = atoi((char*) dlc_str);

		cursor++;
		uint8_t data[8];
		substr((char*) decodedCommand, (char*) data, cursor, 8);

		sendCANMessage(dlc, msgID, false, false, data);

	} else if (decodedCommand[cursor] == 'R') {
		cursor++;
		uint8_t messageID_str[9];
		substr((char*) decodedCommand, (char*) messageID_str, cursor, 9);

		int msgID = toInteger(messageID_str, 9);
		cursor += 9;

		uint8_t dlc_str[1];
		substr((char*) decodedCommand, (char*) dlc_str, cursor, 1);
		uint8_t dlc = toInteger(dlc_str, 1);

		sendCANMessage(dlc, msgID, true, false, 0x00);
	} else if (decodedCommand[cursor] == 't') {
		cursor++;
		uint8_t messageID_str[4];
		substr((char*) decodedCommand, (char*) messageID_str, cursor, 4);
		int msgID = toInteger(messageID_str, 4);
		cursor += 4;

		uint8_t dlc_str[1];
		substr((char*) decodedCommand, (char*) dlc_str, cursor, 1);
		uint8_t dlc = toInteger(dlc_str, 1);

		cursor++;
		uint8_t data[8];
		substr((char*) decodedCommand, (char*) data, cursor, 8);

		sendCANMessage(dlc, msgID, false, true, data);

	} else if (decodedCommand[cursor] == 'r') {
		cursor++;
		uint8_t messageID_str[4];
		substr((char*) decodedCommand, (char*) messageID_str, cursor, 4);
		int msgID = toInteger(messageID_str, 4);
		cursor += 4;

		uint8_t dlc_str[1];
		substr((char*) decodedCommand, (char*) dlc_str, cursor, 1);
		uint8_t dlc = atoi((char*) dlc_str);

		sendCANMessage(dlc, msgID, true, true, 0x00);
	}

}

void processBitRateCommand() {
	uint8_t bitrateSrt[4];
	substr((char*) decodedCommand, (char*) bitrateSrt, 1, 4);
	int bitRate = toInteger(bitrateSrt, 4);

	bool idetified = false;

	switch (bitRate) {
	case 10:
		hcan1.Init.Prescaler = 200;
		idetified = true;
		break;
	case 20:
		hcan1.Init.Prescaler = 100;
		idetified = true;
		break;
	case 50:
		hcan1.Init.Prescaler = 40;
		idetified = true;
		break;
	case 100:
		hcan1.Init.Prescaler = 20;
		idetified = true;
		break;
	case 125:
		hcan1.Init.Prescaler = 16;
		idetified = true;
		break;
	case 250:
		hcan1.Init.Prescaler = 8;
		idetified = true;
		break;
	case 500:
		hcan1.Init.Prescaler = 4;
		idetified = true;
		break;
	case 1000:
		hcan1.Init.Prescaler = 2;
		idetified = true;
		break;
	}
	if (idetified) {
		if (HAL_CAN_DeInit(&hcan1) != HAL_OK) {
			ErrorAppHandler();
		}
		if (HAL_CAN_Init(&hcan1) != HAL_OK) {
			ErrorAppHandler();
		}
		setCANFilterAcceptAll();
		if (HAL_CAN_Start(&hcan1) != HAL_OK) {
			ErrorAppHandler();
		}
		if (snifferAtivityStatus != SNIFFER_STOPPED) {
			if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING)
					!= HAL_OK) {
				ErrorAppHandler();
			}
		}
	}
}

void processLoopBackModeCommand() {
	uint8_t mode[2];
	bool idetified = false;
	substr((char*) decodedCommand, (char*) mode, 1, 2);
	if (!strcmp((char*) mode, "LB")) {
		hcan1.Init.Mode = CAN_MODE_LOOPBACK;
		idetified = true;
	} else if (!strcmp((char*) mode, "SM")) {
		hcan1.Init.Mode = CAN_MODE_SILENT;
		idetified = true;
	} else if (!strcmp((char*) mode, "NM")) {
		hcan1.Init.Mode = CAN_MODE_NORMAL;
		idetified = true;
	} else if (!strcmp((char*) mode, "SL")) {
		hcan1.Init.Mode = CAN_MODE_SILENT_LOOPBACK;
		idetified = true;
	}
	if (idetified) {
		if (HAL_CAN_DeInit(&hcan1) != HAL_OK) {
			ErrorAppHandler();
		}
		if (HAL_CAN_Init(&hcan1) != HAL_OK) {
			ErrorAppHandler();
		}
		setCANFilterAcceptAll();
		if (HAL_CAN_Start(&hcan1) != HAL_OK) {
			ErrorAppHandler();
		}
		if (snifferAtivityStatus != SNIFFER_STOPPED) {
			if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING)
					!= HAL_OK) {
				ErrorAppHandler();
			}
		}
	}
}

void processActivitySniferComand() {
	uint8_t activityMode[2];
	substr((char*) decodedCommand, (char*) activityMode, 1, 3);
	if (!strcmp((char*) activityMode, "ON_")) {
		if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
			ErrorAppHandler();
		}
		snifferAtivityStatus = SNIFFER_ACTIVE;
	}
	else if (!strcmp((char*) activityMode, "OFF")) {
		snifferAtivityStatus = SNIFFER_STOPPED;
		if (HAL_CAN_DeactivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
			ErrorAppHandler();
		}
	}
}
void processFilterCommand(){
	uint8_t  cursor = 1;
	uint8_t  idType[1];
	uint8_t  filterBankStr[2];
	substr((char*) decodedCommand, (char*) idType, cursor, 1);
	// We have a filter for Standard IDs
	if (!strcmp((char*) idType, "S")) {
		cursor += 1;
		substr((char*) decodedCommand, (char*) filterBankStr, cursor, 2);
		cursor += 2;
		int filterBank = toInteger(filterBankStr, 2);
		uint8_t messageID_str[4];
		substr((char*) decodedCommand, (char*) messageID_str, cursor, 4);
		int msgID = toInteger(messageID_str, 4);
		setCANFilterAcceptSingleStandardID(filterBank, msgID);

	}
	// We have a filter for Extended IDs
	else if (!strcmp((char*) idType, "E")) {
		cursor += 1;
		substr((char*) decodedCommand, (char*) filterBankStr, cursor, 2);
		int filterBank = toInteger(filterBankStr, 2);
		cursor += 2;
		uint8_t messageID_str[9];
		substr((char*) decodedCommand, (char*) messageID_str, cursor, 9);
		int msgID = toInteger(messageID_str, 9);
		setCANFilterAcceptSingleExtendedID(filterBank, msgID);
	}
	else {
		return;
	}
}


void processRebootCommand() {
	NVIC_SystemReset();
}

void processComand(void) {

	cobs_decode_result resutlt = cobs_decode(decodedCommand, 20,
			decuedComand.commnddBuff, decuedComand.commandSize);

	if (resutlt.status == COBS_DECODE_OK) {

		if (decodedCommand[0] == 'A') {
			// Change
			processActivitySniferComand();
		}
		if (decodedCommand[0] == 'M') {
			// Send a datagram to the CAN Bus
			processMessageComand();
		} else if (decodedCommand[0] == 'S') {
			// Change bit rate
			processBitRateCommand();
		} else if (decodedCommand[0] == 'N') {
			// Change Mode
			processLoopBackModeCommand();
		}	// Reboot Snifer
		else if (decodedCommand[0] == 'F') {
			processFilterCommand();
		}
		else if (decodedCommand[0] == 'R') {
			processRebootCommand();
		}
	}
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM2) {
		miliseconds++;
		if (snifferAtivityStatus == SNIFFER_STOPPED) {
			HAL_GPIO_TogglePin(PIN4_GPIO_Port, PIN4_Pin);
		}
	}
}

void sendCANStdDataDummyMessage(void){
	uint8_t messageLoadBuffer[8] = {'S','T','D','_','D','A','T','A'};
	sendCANMessage(8, 200, false,true, messageLoadBuffer);
}
void sendCANExtDataDummyMessage(void){
	uint8_t messageLoadBuffer[8] = {'E','X','T','_','D','A','T','A'};
	//uint8_t messageLoadBuffer[8] ={42,0x4c,0x41,0x43,0x4b,0x5f,0x26,0x23};
	sendCANMessage(8,300, false,false, messageLoadBuffer);
}
void sendCANStdRemoteDummyMessage(void){
	sendCANMessage(0,400, true,true, NULL);

}
void sendCANExtRemoteDummyMessage(void){
	sendCANMessage(0,500, true,false, NULL);
}

void generateDummyCANTraffic(uint32_t delay){
	sendCANStdDataDummyMessage();
	HAL_Delay(delay);
	sendCANExtDataDummyMessage();
	HAL_Delay(delay);
	sendCANStdRemoteDummyMessage();
	HAL_Delay(delay);
	sendCANExtRemoteDummyMessage();
	HAL_Delay(delay);
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
  MX_TIM2_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
	// Enable interrupt for to be able to receiving data via UART
	HAL_UART_Receive_IT(&huart3, rxUARTBuff, 1);
	// Set CAN Filter to receive all messages
	setCANFilterAcceptAll();

	//??μ½λ
	HAL_CAN_Start(&hcan1);
	if (snifferAtivityStatus != SNIFFER_STOPPED) {
		HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
	}

	/* ?λ²κΉ μ½λ SNIFFER_ACTIVITY ??λ₯? λ§λ€κΈ? ??΄? */
//	HAL_CAN_Start(&hcan1);
//	if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
//		ErrorAppHandler();  // ??¬ μ²λ¦¬
//	}

	// Initialize EncuedCANMsg
	q_init(&canMsgQueue, sizeof(EncuedCANMsg), CAN_MSSG_QUEUE_SIZE,
			IMPLEMENTATION, false);
	// Initialize EnueuedCommand
	q_init(&commandQueue, sizeof(EnueuedCommand), CMD_QUEUE_SIZE,
			IMPLEMENTATION, true);
	// Displays we are starting the main loop
	blueLEDIndicator();
	HAL_TIM_Base_Start_IT(&htim2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

		// Use this function to make it a a dummy traffic generator
		//generateDummyCANTraffic(10);

		if (q_pop(&commandQueue, &decuedComand)) {
			processComand();
		}
		if (q_pop(&canMsgQueue, &decuedCANMssg)) {
			processCANMsg();
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 128;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
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
  hcan1.Init.Prescaler = 8;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_13TQ;
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
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */
  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */
  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 32-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 1000-1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */
  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */
  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */
  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */
  /* USER CODE END USART3_Init 2 */

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, PIN3_Pin|PIN2_Pin|PIN4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, PIN5_Pin|PIN1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GREEN_LED_Pin|ORANGE_LED_Pin|RED_LED_Pin|BLUE_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PIN3_Pin PIN2_Pin PIN4_Pin */
  GPIO_InitStruct.Pin = PIN3_Pin|PIN2_Pin|PIN4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PIN5_Pin PIN1_Pin */
  GPIO_InitStruct.Pin = PIN5_Pin|PIN1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : GREEN_LED_Pin ORANGE_LED_Pin RED_LED_Pin BLUE_LED_Pin */
  GPIO_InitStruct.Pin = GREEN_LED_Pin|ORANGE_LED_Pin|RED_LED_Pin|BLUE_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

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
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
