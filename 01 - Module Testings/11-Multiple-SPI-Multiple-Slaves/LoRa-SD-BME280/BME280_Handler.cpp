#include "BME280_Handler.h"


void update_BME280_settings(Adafruit_BME280* bme) {
  bme->setSampling( BME280_userSettings.Power_Mode,                 // Overwrite BME280 power mode
                    BME280_userSettings.Temperature_Oversampling,   // Overwrite BME280 temperature oversampling
                    BME280_userSettings.Pressure_Oversampling,      // Overwrite BME280 pressure oversampling
                    BME280_userSettings.Humidity_Oversampling,      // Overwrite BME280 humidity oversampling
                    BME280_userSettings.Filter_Coefficient          // Overwrite BME280 IIR filter coefficient
                  );
}

void read_BME280(Adafruit_BME280 *bme, bool* controlFlag) {
  if (BME280_userSettings.Power_Mode == Adafruit_BME280::MODE_FORCED) {
    bme->takeForcedMeasurement();
  }
   
  BME280_dataStorage.temperature = bme->readTemperature();         // Read temperature data in degree Celsius
  BME280_dataStorage.pressure    = bme->readPressure() / 100.0F;   // Read barometric data in hPa
  BME280_dataStorage.humidity    = bme->readHumidity();            // Read relative humidity data in %RH

  *controlFlag = false;
}


BME280_settings BME280_userSettings;  // Holder for sensor settings
BME280_Data BME280_dataStorage;       // BME280 data storage
