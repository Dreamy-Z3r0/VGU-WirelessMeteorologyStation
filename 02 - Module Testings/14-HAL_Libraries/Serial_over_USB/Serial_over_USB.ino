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
  while (!Serial);

  pinMode(PC13, OUTPUT);
}

void loop() {
  static bool LED_STATE = false;
  
  Serial.println("Hello world!");

  digitalWrite(PC13, LED_STATE);
  LED_STATE = !LED_STATE;
  
  HAL_Delay(1000);
}
