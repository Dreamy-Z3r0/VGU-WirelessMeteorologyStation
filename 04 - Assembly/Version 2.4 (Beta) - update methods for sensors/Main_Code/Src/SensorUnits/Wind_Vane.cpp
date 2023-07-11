#include "../../Inc/SensorUnits/Wind_Vane.h"


WindVane_Control WindVane;

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

bool first_run = true;


/* Global variables used in calculations */

// Maximum value of ADC output by the resolution
uint16_t ADC_MAX_VALUE = 4095;

// Supply voltage of the wind vane (Vcc) and ADC reference voltage of the microcontroller (AVref)
float Vcc = 3.3,      // Initial value for Vcc
      AVref = 3.3;    // Initial value for AVref


/*********************
 *** Contructor(s) ***
 *********************/

// Initiate a class with pre-defined array size for temporary data storages
WindVane_Control::WindVane_Control(unsigned int storage_size) {
  this->storage_size = storage_size;
  
  clear_readFlag();
  set_standbyFlag();
}

// Accept input pin parameter and pre-defined array size for temporary data storages
WindVane_Control::WindVane_Control(uint32_t ADC_input_pin, unsigned int storage_size) {
  set_SensorPin(ADC_input_pin);
  this->storage_size = storage_size;

  clear_readFlag();
  set_standbyFlag();
}


/**********************
 *** Initialization ***
 **********************/

// Initiate the ADC
void WindVane_Control::init(void) {
  MX_DMA_Init();
  MX_ADC1_Init(EXTERNAL_INPUT_SIGNAL);
  HAL_ADCEx_Calibration_Start(&hadc1);
}


/**************************
 *** Reading operations ***
 **************************/

// Initiate a read operation of wind direction
void WindVane_Control::update_sensor_data(void) {
  if (is_readFlag_set() && is_standbyFlag_set()) {
    clear_standbyFlag();

    sampleReady = false;
    halfComplete = false;

    read_reference();   // Read the ADC reference voltage of the microcontroller

    raw_data = new uint16_t[storage_size];    // Temporary storage for ADC values
    read_raw_ADC(raw_data);    // Take ADC samples
  } else if (sampleReady) {    // A reading routine is on-going
    Data_Processing_Routine(raw_data);  // Process for wind direction data, update standbyFlag when done
  }
}

void WindVane_Control::End_Of_Sampling_Routine(bool halfComplete) {
  sampleReady = true;
  this->halfComplete = halfComplete;
}

// Manages all the data-processing sub-routines at the end of a sampling routine
void WindVane_Control::Data_Processing_Routine(uint16_t* raw_data) {
  float voltage_mean_value = 0;
  
  unsigned int n = 0,
               half_storage_size = (unsigned int)(storage_size / 2);
  do {    
    float x = rawData_to_voltage(raw_data+n);   // Convert the ADC value at index n of raw_data[] array to voltage value
                                                                                                
    voltage_mean_value += (x / storage_size);

    n += 1;
    if (n == half_storage_size) 
      while (halfComplete);
  } while (n < storage_size);

  delete[] raw_data;   // Free up the heap

  Wind_Direction_Instance(voltage_mean_value);
}

// Read the reference voltage for ADC
void WindVane_Control::read_reference(void) {
  // Switch ADC to internal channel(s)
  MX_ADC1_Init(INTERNAL_REFERENCE_VOLTAGE);

  // Initiate sampling of internal reference voltage
  HAL_ADC_Start(&hadc1);  
  HAL_ADC_PollForConversion(&hadc1, 100);
    
  // Fetch conversion result (dummie value)
  uint16_t raw_VREFINT = HAL_ADC_GetValue(&hadc1);

  // Take real ADC value for AVref
  HAL_ADC_PollForConversion(&hadc1, 100);
  raw_VREFINT = HAL_ADC_GetValue(&hadc1);
  
  // Calculate reference voltage
  AVref = (VREFINT * (ADC_MAX_VALUE+1) / raw_VREFINT) / 1000.0;   // Take the AVref
  Vcc = AVref;  // No VADD and VASS, so Vcc = VAref
}

// Sample input signal from the wind vane
void WindVane_Control::read_raw_ADC(uint16_t* storage) {  
  // Switch to external channel(s)
  MX_ADC1_Init(EXTERNAL_INPUT_SIGNAL);

  // Sample then discard the first value (dummie value)
  HAL_ADC_Start(&hadc1);  
  HAL_ADC_PollForConversion(&hadc1, 100);

  // Initiate ADC sampling in non-blocking mode with DMA
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)storage, (uint32_t)storage_size);
}

// Convert an instance of raw ADC values to voltage value
float WindVane_Control::rawData_to_voltage(uint16_t* raw_data) {
  return ((AVref * (*raw_data)) / ADC_MAX_VALUE);
}

// Take the final estimation of the wind direction
void WindVane_Control::Wind_Direction_Instance(float V_in) {
  uint8_t output_index = 0;
  float V_dif;
  
  for (uint8_t index = 0; index < NumberOfDirection; index += 1) {
    float V_dir_ref = Vcc * R_in_given[index] / (R + R_in_given[index]);
    float temp_V_dif = abs(V_in - V_dir_ref);
    
    if (0 != index) {  
      if (temp_V_dif < V_dif) {
        V_dif = temp_V_dif;
        output_index = index;
      }
    }
    else {
      V_dif = temp_V_dif;
    }
  }

  windDir = 22.5 * output_index; 

  // Reset flag values
  set_standbyFlag();
  sampleReady = false;
  halfComplete = false;
}


/***********************************
 *** Data-returning operation(s) ***
 ***********************************/

// Returns the latest wind direction value
void WindVane_Control::read_sensor_data(float *external_storage) {    // Returns the latest wind direction value
  if (is_readFlag_set() && is_standbyFlag_set()) {    // Double-check status flags to avoid error(s)
    *external_storage = windDir;    // Return wind direction data
    clear_readFlag();   // End of reading routine
  }
}


/*********************************************
 *** HAL settings generated by STM32CubeMX ***
 *********************************************/

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
    sConfig.SamplingTime = ADC_SAMPLETIME_7CYCLES_5;
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
    sConfig.SamplingTime = ADC_SAMPLETIME_7CYCLES_5;
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

// Callback function when DMA has filled half of the ADC buffer
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) {
  WindVane.End_Of_Sampling_Routine(true);
}

// Callback function when DMA has filled the ADC buffer
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
  WindVane.End_Of_Sampling_Routine();
}