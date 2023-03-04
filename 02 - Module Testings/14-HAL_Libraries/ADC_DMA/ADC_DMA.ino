#include "hal_conf_extra.h"

#define CALX_TEMP 25      // Reference temperature (25ºC)
#define V25       1430    // Vnternal voltage signal (V_sense) at reference temperature in mV
#define AVG_SLOPE 4300    // Average slope for curve between temperature and V_sense
#define VREFINT   1.20    // Internal reference voltage in V

uint16_t ADC_MAX_VALUE = 4095;

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

#define quantity 50

#define R 3.3

uint16_t raw[quantity];
bool output_ready = false;

bool first_run = true;
float V_ref, Vcc;

void setup() {
  Serial.begin(9600);
  delay(1000);
  
  MX_DMA_Init();
  
  MX_ADC1_Init(false);
  HAL_ADCEx_Calibration_Start(&hadc1);

  HAL_ADC_Start(&hadc1); 
  HAL_ADC_PollForConversion(&hadc1, 100);

  uint16_t raw_VREFINT = HAL_ADC_GetValue(&hadc1);
  V_ref = (VREFINT * (ADC_MAX_VALUE+1)) / raw_VREFINT;
  Serial.printf("V_ref = %.2f V\n", V_ref);

  while (!Serial.available());
  String input_Vcc = Serial.readStringUntil('\n');
  Vcc = input_Vcc.toFloat();
  Serial.printf("Vcc = %.2f V\n", Vcc);
}

void loop() {
  if (Serial.available()) {
    String input_command = Serial.readStringUntil('\n');
    if (input_command.equals("go")) {
      Serial.println("\nStart sampling...");
      output_ready = false;
      MX_ADC1_Init(true);

      HAL_ADC_Start_DMA(&hadc1, (uint32_t*)raw, 50);
      while(!output_ready);
      output_ready = false;

      Serial.printf("Raw data:\n");
      for (unsigned int index = 0; index < quantity; index += 1) {
        Serial.printf("%d", raw[index]);
        if ((index+1) % 10 == 0) {
          Serial.print("\n");
        } else {
          Serial.print(" ");
        }
      }

      Serial.printf("Voltage value (by Vcc):\n");
      for (unsigned int index = 0; index < quantity; index += 1) {
        float V_in = Vcc * ((1.0 * raw[index]) / ADC_MAX_VALUE);
        Serial.printf("%.2fV", V_in);
        if ((index+1) % 10 == 0) {
          Serial.print("\n");
        } else {
          Serial.print(" ");
        }
      }

      Serial.printf("Voltage value (by V_ref):\n");
      for (unsigned int index = 0; index < quantity; index += 1) {
        float V_in = V_ref * ((1.0 * raw[index]) / ADC_MAX_VALUE);
        Serial.printf("%.2fV", V_in);
        if ((index+1) % 10 == 0) {
          Serial.print("\n");
        } else {
          Serial.print(" ");
        }
      }
    }
  }
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
  /* USER CODE BEGIN ADC1_MspDeInit 0 */

  /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC1_CLK_DISABLE();

    /**ADC1 GPIO Configuration
    PB1     ------> ADC1_IN9
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_1);

    /* ADC1 DMA DeInit */
    HAL_DMA_DeInit(hadc->DMA_Handle);
  /* USER CODE BEGIN ADC1_MspDeInit 1 */

  /* USER CODE END ADC1_MspDeInit 1 */
  }

}

/**
  * @brief This function handles DMA1 channel1 global interrupt.
  */
extern "C" void DMA1_Channel1_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel1_IRQn 0 */

  /* USER CODE END DMA1_Channel1_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_adc1);
  /* USER CODE BEGIN DMA1_Channel1_IRQn 1 */

  /* USER CODE END DMA1_Channel1_IRQn 1 */
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(bool input_type)
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
  if (!input_type) {
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
  } else {
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

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    output_ready = true;
}
