/* STM32F1xx specific HAL configuration options. */
#if __has_include("hal_conf_custom.h")
#include "hal_conf_custom.h"
#else
#if __has_include("hal_conf_extra.h")
#include "hal_conf_extra.h"
#endif
#include "stm32f1xx_hal_conf_default.h"
#endif

ADC_HandleTypeDef hadc2;
uint16_t raw[50];
uint8_t arr_index = 0;

void setup() {
  Serial.begin(9600);

  MX_ADC2_Init();
}

void loop() {
  unsigned long t1, t2;

  t1 = micros();
  for (arr_index = 0; arr_index < 50; arr_index += 1) {
    HAL_ADC_Start(&hadc2);
    HAL_ADC_PollForConversion(&hadc2, 1000);
    raw[arr_index] = HAL_ADC_GetValue(&hadc2);
  }
  t2 = micros();

  Serial.print("ADC readout: ");

  for (uint8_t i = 0; i < 50; i += 1) {
    if (0 == i%10) Serial.printf("\n");
    Serial.printf(" %d", raw[i]);
  }

  Serial.printf("\nRead time: %d us\n\n", t2 - t1);

//  Serial.print("ADC readout: ");
//  Serial.println(raw);
//  Serial.print("V = ");
//  Serial.print(3.3*raw/4095);
//  Serial.println(" V\n");
  
  HAL_Delay(1000);
}

static void MX_ADC2_Init(void)
{

  /* USER CODE BEGIN ADC2_Init 0 */

  /* USER CODE END ADC2_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC2_Init 1 */

  /* USER CODE END ADC2_Init 1 */

  /** Common config
  */
  hadc2.Instance = ADC2;
  hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc2.Init.ContinuousConvMode = DISABLE;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    while (1);
  }
}
