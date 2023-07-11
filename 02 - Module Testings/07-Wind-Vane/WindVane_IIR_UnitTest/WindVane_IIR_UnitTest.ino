#include <SPI.h>
#include <SD.h>

#include "hal_conf_extra.h"

#define CALX_TEMP 25      // Reference temperature (25ºC)
#define V25       1430    // Vnternal voltage signal (V_sense) at reference temperature in mV
#define AVG_SLOPE 4300    // Average slope for curve between temperature and V_sense
#define VREFINT   1200    // Internal reference voltage in mV

uint16_t ADC_MAX_VALUE = 4095;

#define numberOfDataPoints 38

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

bool first_run = true;

uint16_t* raw_data;
uint32_t raw;
float V_ref, V_in, R_read, direction_read;
bool output_ready = false;

float R_pullup = 3.3E3;
float R_internal[] = {  33E3, 6.57E3, 8.2E3,     891,   1E3,     688,  2.2E3,  1.41E3, 
                       3.9E3, 3.14E3,  16E3, 14.12E3, 120E3, 42.12E3, 64.9E3, 21.88E3 };
float direction_degrees[] = {   0,  22.5,  45,  67.5,  90, 112.5, 135, 157.5, 
                              180, 202.5, 225, 247.5, 270, 292.5, 315, 337.5 };

#define card_NCSS PA4
File logger;

double b0 = 1,
       b1 = 1,
       b2 = 0,
       a1 = -0.78680917290464136000,
       a2 = 0;

void setup() {
  Serial.begin(9600);
  delay(1000);

  SPI.setMOSI(PA7);
  SPI.setMISO(PA6);
  SPI.setSCLK(PA5);
//  SPI.setSSEL(PA4);
  
  // Initiate communication with microSD card
  if (!SD.begin(card_NCSS)) {
    Serial.println("Cannot initialise microSD card.");
    while (1);
  }

  MX_DMA_Init();
  
  MX_ADC1_Init(false);
  HAL_ADCEx_Calibration_Start(&hadc1);

  HAL_ADC_Start(&hadc1); 
  HAL_ADC_PollForConversion(&hadc1, 100);

  uint16_t raw_VREFINT = HAL_ADC_GetValue(&hadc1);
  V_ref = (VREFINT * (ADC_MAX_VALUE+1)) / raw_VREFINT;
  V_ref /= 1000;
  Serial.printf("V_ref = %.2f V\n", V_ref);

  // Write the first line
//  String FirstLine = "Input direction,Read direction,V_in,R_read";
  logger = SD.open("logger.csv", FILE_WRITE);
  if (logger) {
//    logger.println(FirstLine);
    logger.close();
  } else {
    Serial.println("Error openning logger file (setup)");
    while (1);
  }

  Serial.println("End of setup()\n");

  output_ready = false;
  MX_ADC1_Init(true);
  HAL_ADC_Start_DMA(&hadc1, &raw, 1);
  while (!output_ready);
  Serial.printf("Dummy reading 1: %d\n", raw);

  output_ready = false;
  MX_ADC1_Init(true);
  HAL_ADC_Start_DMA(&hadc1, &raw, 1);
  while (!output_ready);
  Serial.printf("Dummy reading 2: %d\n", raw);

  output_ready = false;
  MX_ADC1_Init(true);
  HAL_ADC_Start_DMA(&hadc1, &raw, 1);
  while (!output_ready);
  Serial.printf("Dummy reading 3: %d\n\n", raw);
}

void loop() {
  if (Serial.available()) {
    String serialInput = Serial.readStringUntil('\n');

    // Take a dummie ADC value
    output_ready = false;
    MX_ADC1_Init(true);
    HAL_ADC_Start_DMA(&hadc1, &raw, 1);
    while (!output_ready);
    Serial.printf("Cycle dummy reading: %d\n", raw);

    // Sampling real values
    output_ready = false;
    raw_data = new uint16_t[numberOfDataPoints];
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)raw_data, (uint32_t)numberOfDataPoints);
    while (!output_ready);

    Data_Processing_Routine(raw_data);  // -> obtain V_in
    R_read = R_pullup * (V_in / (V_ref - V_in));

    uint8_t index = 0;
    uint8_t count = 1;
    float dif;
    for (uint8_t i = 0; i < 16; i += 1) {
      if (0 == i) {
        if (R_read >= R_internal[i]) dif = R_read - R_internal[i];
        else dif = R_internal[i] - R_read;
      } else {
        float dif_temp;
        if (R_read >= R_internal[i]) dif_temp = R_read - R_internal[i];
        else dif_temp = R_internal[i] - R_read;

        if (dif_temp < dif) {
          dif = dif_temp;
          index = i;
          count = 1;
        } else if (dif_temp == dif) {
          count += 1;
        }
      }
    }
      
    direction_read = direction_degrees[index];

    logger = SD.open("logger.csv", FILE_WRITE);
    if (logger) {
      logger.print(serialInput);
      logger.print(",");
      logger.print(direction_read, 1);
      logger.print(",");
      logger.print(V_in, 4);
      logger.print(",");
      logger.println(R_read);
      logger.close();
    } else {
      Serial.println("Error openning logger file.");
      while (1);
    }

    Serial.println("Finished sampling.");
    
    Serial.print("SD updated for direction ");
    Serial.println(serialInput);
    Serial.print("Read direction: ");
    Serial.println(direction_read, 1);
    Serial.print("V_in: ");
    Serial.println(V_in);    
    Serial.println();
  }
}

void Data_Processing_Routine(uint16_t* raw_data_value) {
  float x[3],   // Stores voltage from raw ADC data / inputs for IIR filter
        y[3];   // Stores outputs of IIR filter

  float voltage_mean_value = 0;
  unsigned int n = 0;

  do {
    x[0] = rawData_to_voltage(raw_data_value+n);
    IIR_Filter(x, y);

    if (n >= (numberOfDataPoints-16)) 
      voltage_mean_value += (y[0] / 16);

    n += 1;
  } while (n < numberOfDataPoints);

  delete[] raw_data_value;
  V_in = voltage_mean_value;
}

float rawData_to_voltage(uint16_t* raw_data_value) {
  return ((V_ref * (*raw_data_value)) / ADC_MAX_VALUE);
}

void IIR_Filter(float *x, float *y) {
  // IIR Filter direct form II Transposed output function: (highest order: 2)
  //    y[n] = (b0 * x[n] + b1 * x[n-1] + b2 * x[n-2]) - (a1 * y[n-1] + a2 * y[n-2])

  y[0] = (float)(b0*x[0] + b1*x[1] + b2*x[2] - (a1*y[1] + a2*y[2]));

  x[2] = x[1];
  x[1] = x[0];

  y[2] = y[1];
  y[1] = y[0];
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
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
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
