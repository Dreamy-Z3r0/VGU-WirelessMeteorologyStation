#include "Thermometer_Hygrometer_Barometer.h"


void update_BME280_settings(Adafruit_BME280* bme, BME280_settings* updated_settings) {
  bme->setSampling( updated_settings->Power_Mode,                 // Overwrite BME280 power mode
                    updated_settings->Temperature_Oversampling,   // Overwrite BME280 temperature oversampling
                    updated_settings->Pressure_Oversampling,      // Overwrite BME280 pressure oversampling
                    updated_settings->Humidity_Oversampling,      // Overwrite BME280 humidity oversampling
                    updated_settings->Filter_Coefficient          // Overwrite BME280 IIR filter coefficient
                  );
}

void read_BME280(Adafruit_BME280* bme, BME280_settings* settings, BME280_Data* storage, bool* controlFlag) {
  if (settings->Power_Mode == Adafruit_BME280::MODE_FORCED) {
    bme->takeForcedMeasurement();
  }
  
  storage->temperature = bme->readTemperature();         // Read temperature data in degree Celsius
  storage->pressure    = bme->readPressure() / 100.0F;   // Read barometric data in hPa
  storage->humidity    = bme->readHumidity();            // Read relative humidity data in %RH

  *controlFlag = false;
}
