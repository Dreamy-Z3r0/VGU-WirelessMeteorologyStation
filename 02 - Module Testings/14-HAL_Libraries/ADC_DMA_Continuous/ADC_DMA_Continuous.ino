#include "Wind_Vane.h"

void setup() {
  WindVane.init();

  
}

void loop() {
  WindVane.update_sensor_data();
  while (!WindVane.is_Data_Ready());
  delay(5000);
}
