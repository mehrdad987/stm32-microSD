/* USER CODE BEGIN Header */
/* https://github.com/mehrdad987/stm32-NMEA2000 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32f1xx_hal.h"
#include "stdio.h"
#include "string.h"
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
CAN_HandleTypeDef hcan;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
CAN_RxHeaderTypeDef rxHeader; //CAN Bus Transmit Header
CAN_TxHeaderTypeDef txHeader; //CAN Bus Receive Header
CAN_FilterTypeDef sFilterConfig;
uint8_t rxData[8];
uint8_t txData[8];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// Define a structure to hold PGN and its corresponding name
typedef struct {
  uint32_t pgn;
  const char *name;
} PGN_Name;

// Lookup table mapping PGNs to their names
PGN_Name pgnNames[] = {
    {129029, "GNSS Position Data"},
    {130306, "Wind Data"},
    {126992L,"SystemTime"},
    {127245L,"Rudder"},
    {127250L,"Heading"},
    {127257L,"Attitude"},
    {127488L,"EngineRapid"},
    {127489L,"EngineDynamicParameters"},
    {127493L,"TransmissionParameters"},
    {127497L,"TripFuelConsumption"},
    {127501L,"BinaryStatus"},
    {127505L,"FluidLevel"},
    {127506L,"DCStatus"},
    {127513L,"BatteryConfigurationStatus"},
    {128259L,"Speed"},
    {128267L,"WaterDepth"},
    {129026L,"COGSOG"},
    {129029L,"GNSS"},
    {129033L,"LocalOffset"},
    {129045L,"UserDatumSettings"},
    {129540L,"GNSSSatsInView"},
    {130310L,"OutsideEnvironmental"},
    {130312L,"Temperature"},
    {130313L,"Humidity"},
    {130314L,"Pressure"},
    {130316L,"TemperatureExt"},
  // Add more PGNs and their names as needed
};

// Function to get the name of a PGN
const char* getPGNName(uint32_t pgn) {
  for (int i = 0; i < sizeof(pgnNames) / sizeof(pgnNames[0]); i++) {
    if (pgnNames[i].pgn == pgn) {
      return pgnNames[i].name;
    }
  }
  return "Unknown PGN";  // Return a default name for unknown PGNs
}

void sendNMEA2000Message(uint32_t pgn, uint8_t *data, uint8_t len) {
  CAN_TxHeaderTypeDef txHeader;
  txHeader.StdId = pgn;
  txHeader.IDE = CAN_ID_STD;
  txHeader.RTR = CAN_RTR_DATA;
  txHeader.DLC = len;

  if (HAL_CAN_AddTxMessage(&hcan, &txHeader, data, NULL) != HAL_OK) {
    Error_Handler();
  }
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
  MX_CAN_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  // Initialize CAN peripheral
   hcan.Instance = CAN1;
   hcan.Init.Mode = CAN_MODE_NORMAL;
   hcan.Init.AutoBusOff = ENABLE;
   hcan.Init.AutoRetransmission = ENABLE;
   hcan.Init.AutoWakeUp = DISABLE;
   hcan.Init.ReceiveFifoLocked = DISABLE;
   hcan.Init.TimeTriggeredMode = DISABLE;
   hcan.Init.TransmitFifoPriority = DISABLE;
   hcan.Init.Prescaler = 16;
   hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
   hcan.Init.TimeSeg1 = CAN_BS1_3TQ;
   hcan.Init.TimeSeg2 = CAN_BS2_5TQ;
   HAL_CAN_Init(&hcan);

   // Start the CAN peripheral
   if (HAL_CAN_Start(&hcan) != HAL_OK) {
     Error_Handler();
   }

   // Configure CAN filter to receive all messages

   sFilterConfig.FilterBank = 0;
   sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
   sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
   sFilterConfig.FilterIdHigh = 0x0000;
   sFilterConfig.FilterIdLow = 0x0000;
   sFilterConfig.FilterMaskIdHigh = 0x0000;
   sFilterConfig.FilterMaskIdLow = 0x0000;
   sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
   sFilterConfig.FilterActivation = ENABLE;
   sFilterConfig.SlaveStartFilterBank = 14;
   if (HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK) {
     Error_Handler();
   }

   // Start the CAN reception
   if (HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
     Error_Handler();
   }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  printf("// ...initiate...");
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  // Check for pending messages in the receive FIFO
	        if (HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0) > 0) {


	          if (HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &rxHeader, rxData) == HAL_OK) {
	            // Output the received NMEA 2000 message to the serial interface
	            char buffer[50];
	            sprintf(buffer, "Received NMEA 2000 message: %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
	                    rxData[0], rxData[1], rxData[2], rxData[3], rxData[4], rxData[5], rxData[6], rxData[7]);
	            HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);

	            // Decode the received NMEA 2000 message
	            decodeNMEA2000Message(rxData);
	          }
	        }

	        // Send a sample NMEA 2000 message
	        uint8_t sampleData[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
	        sendNMEA2000Message(130306, sampleData, sizeof(sampleData));

	        HAL_Delay(1000);  // Delay for 1 second
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
  * @brief CAN Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 9;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_8TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_7TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

  /* USER CODE END CAN_Init 2 */

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
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void decodeNMEA2000Message(uint8_t *data) {
  // Example decoding of NMEA 2000 message
  uint8_t priority = (data[0] >> 2) & 0x07;  // Extract priority from the first byte
  uint32_t PGN = ((uint32_t)data[1] << 16) | ((uint32_t)data[2] << 8) | data[3];  // Extract PGN from bytes 2, 3, and 4
  uint8_t destination = data[4];  // Extract destination address
  uint8_t source = data[5];  // Extract source address
  // ... Decode other relevant data fields from the message

  // Get the name of the PGN
  const char* pgnName = getPGNName(PGN);

  // Print the decoded information including the PGN name to the serial interface
  char buffer[150];
  sprintf(buffer, "Decoded NMEA 2000 message - Priority: %d, PGN: %lu (%s), Destination: %d, Source: %d\r\n",
          priority, PGN, pgnName, destination, source);
  HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
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
