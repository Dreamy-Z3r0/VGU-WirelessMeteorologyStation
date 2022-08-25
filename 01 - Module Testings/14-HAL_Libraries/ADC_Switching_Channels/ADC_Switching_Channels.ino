/* STM32F1xx specific HAL configuration options. */
#if __has_include("hal_conf_custom.h")
#include "hal_conf_custom.h"
#else
#if __has_include("hal_conf_extra.h")
#include "hal_conf_extra.h"
#endif
#include "stm32f1xx_hal_conf_default.h"
#endif

/* Constants from datasheet */
#define CALX_TEMP 25      // Reference temperature (25ºC)
#define V25       1430    // Vnternal voltage signal (V_sense) at reference temperature in mV
#define AVG_SLOPE 4300    // Average slope for curve between temperature and V_sense
#define VREFINT   1200    // Internal reference voltage in mV

enum ADC_INPUT_TYPE {INTERNAL_REFERENCE_VOLTAGE, EXTERNAL_INPUT_SIGNAL};

const uint16_t ADC_MAX_VALUE = 4095;
#define BUFFER_SIZE 50

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

float AVref = 3.3;

uint16_t buf[BUFFER_SIZE];
bool conversion_complete = false;

bool first_run = true;

void setup() {
  Serial.begin(9600);
  MX_DMA_Init();

  MX_ADC1_Init(INTERNAL_REFERENCE_VOLTAGE);
  HAL_ADCEx_Calibration_Start(&hadc1);
}

void loop() {  
  // Read reference voltage
  MX_ADC1_Init(INTERNAL_REFERENCE_VOLTAGE);

  HAL_ADC_Start(&hadc1);
  HAL_ADC_PollForConversion(&hadc1, 100);

  uint16_t raw_VREFINT = HAL_ADC_GetValue(&hadc1);
  
  AVref = (VREFINT * (ADC_MAX_VALUE+1) / raw_VREFINT) / 1000.0;   // Take the AVref
  Serial.printf("raw = %d\n", raw_VREFINT);
  Serial.printf("AVref = %.3fV\n\n", AVref);

  // Read ADC on PB1
  MX_ADC1_Init(EXTERNAL_INPUT_SIGNAL);

  HAL_ADC_Start(&hadc1);
  HAL_ADC_PollForConversion(&hadc1, 100);

  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)buf, BUFFER_SIZE);
  while (!conversion_complete) {
    // Do nothing
  }

  Serial.printf("Raw ADC:");
  for (uint8_t i = 0; i < BUFFER_SIZE; i += 1) {
    if (0 == i % 10) Serial.println();
    Serial.printf(" %d", buf[i]);
  }

  Serial.printf("\nVoltage:");
  for (uint8_t i = 0; i < BUFFER_SIZE; i += 1) {
    if (0 == i % 10) Serial.println();
    Serial.printf(" %.3fV", AVref*buf[i]/ADC_MAX_VALUE);
  }

  Serial.printf("\n\n\n");  

  HAL_Delay(1000);
}

/**
* @brief ADC MSP Initialization
* This function configures the hardware resources used in this example
* @param hadc: ADC handle pointer
* @retval None
*/
extern "C" void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hadc->Instance==ADC1)
  {
    /* Peripheral clock enable */
    __HAL_RCC_ADC1_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**ADC1 GPIO Configuration
    PB1     ------> ADC1_IN9
    */
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* ADC1 DMA Init */
    /* ADC1 Init */
    hdma_adc1.Instance = DMA1_Channel1;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_NORMAL;
    hdma_adc1.Init.Priority = DMA_PRIORITY_HIGH;
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
    {
      while (1);
    }

    __HAL_LINKDMA(hadc,DMA_Handle,hdma_adc1);
  }

}

/**
* @brief ADC MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hadc: ADC handle pointer
* @retval None
*/
extern "C" void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc)
{
  if(hadc->Instance==ADC1)
  {
    /* Peripheral clock disable */
    __HAL_RCC_ADC1_CLK_DISABLE();

    /**ADC1 GPIO Configuration
    PB1     ------> ADC1_IN9
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_1);

    /* ADC1 DMA DeInit */
    HAL_DMA_DeInit(hadc->DMA_Handle);
  }

}

/**
  * @brief This function handles DMA1 channel1 global interrupt.
  */
extern "C" void DMA1_Channel1_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_adc1);
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(ADC_INPUT_TYPE input_type)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */
  if (!first_run) {
    if (HAL_ADC_DeInit(&hadc1) != HAL_OK)
    {
      while (1);
    }
  }
  else {
    first_run = false;
  }
  /* USER CODE END ADC1_Init 1 */

  
  /* USER CODE BEGIN ADC1_Init 2 */
  if (INTERNAL_REFERENCE_VOLTAGE == input_type) {
    /** Common config
    */
    hadc1.Instance = ADC1;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;
    if (HAL_ADC_Init(&hadc1) != HAL_OK) {
      while (1);
    }

    /** Configure Regular Channel
    */
    sConfig.Channel = ADC_CHANNEL_VREFINT;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
      while (1);
    }
  }
  else if (EXTERNAL_INPUT_SIGNAL == input_type) {
    /** Common config
    */
    hadc1.Instance = ADC1;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.ContinuousConvMode = ENABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;
    if (HAL_ADC_Init(&hadc1) != HAL_OK) {
      while (1);
    }

    /** Configure Regular Channel
    */
    sConfig.Channel = ADC_CHANNEL_9;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
      while (1);
    }
  }
  else {
    // Do nothing
  }
/* USER CODE END ADC1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}


/****************************************************
 *** Callback function(s) required by HAL modules ***
 ****************************************************/

// Callback function when DMA has filled the ADC buffer
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
  conversion_complete = true;
}
