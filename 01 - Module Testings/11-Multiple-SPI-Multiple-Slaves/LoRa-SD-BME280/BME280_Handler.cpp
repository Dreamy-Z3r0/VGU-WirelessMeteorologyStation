#include "BME280_Handler.h"

void read_BME280(Adafruit_BME280 *bme, BME280_Data* storage, bool* controlFlag) {
  storage->temperature = bme->readTemperature();         // Read temperature data in degree Celsius
  storage->pressure    = bme->readPressure() / 100.0F;   // Read barometric data in hPa
  storage->humidity    = bme->readHumidity();            // Read relative humidity data in %RH

  *controlFlag = false;
}
