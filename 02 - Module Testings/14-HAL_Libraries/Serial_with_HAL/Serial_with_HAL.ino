/* STM32F1xx specific HAL configuration options. */
#if __has_include("hal_conf_custom.h")
#include "hal_conf_custom.h"
#else
#if __has_include("hal_conf_extra.h")
#include "hal_conf_extra.h"
#endif
#include "stm32f1xx_hal_conf_default.h"
#endif

void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.printf("Hello world!\n");
  HAL_Delay(1000);
}
