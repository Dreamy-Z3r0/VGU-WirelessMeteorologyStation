#include "Thermometer_Hygrometer_Barometer.h"


BME280_Control::BME280_Control(Adafruit_BME280* bme280_instance) {
  this->bme280_instance = bme280_instance;
  BME280_readFlag = false;
}


void BME280_Control::update_BME280_settings(unsigned long custom_readInterval) {
  BME280_userSettings.readInterval = custom_readInterval;
  
  bme280_instance->setSampling( 
                     BME280_userSettings.Sensor_Mode,                // Overwrite BME280 power mode
                     BME280_userSettings.Temperature_Oversampling,   // Overwrite BME280 temperature oversampling
                     BME280_userSettings.Pressure_Oversampling,      // Overwrite BME280 pressure oversampling
                     BME280_userSettings.Humidity_Oversampling,      // Overwrite BME280 humidity oversampling
                     BME280_userSettings.Filter_Coefficient          // Overwrite BME280 IIR filter coefficient
                   );
}

void BME280_Control::new_SensorMode(Adafruit_BME280::sensor_mode newValue) {
  BME280_userSettings.Sensor_Mode = newValue;
}

void BME280_Control::new_TemperatureOversampling(Adafruit_BME280::sensor_sampling newValue) {
  BME280_userSettings.Temperature_Oversampling = newValue;
}

void BME280_Control::new_PressureOversampling(Adafruit_BME280::sensor_sampling newValue) {
  BME280_userSettings.Pressure_Oversampling = newValue;
}

void BME280_Control::new_HumidityOversampling(Adafruit_BME280::sensor_sampling newValue) {
  BME280_userSettings.Humidity_Oversampling = newValue;
}

void BME280_Control::new_FilterCoefficient(Adafruit_BME280::sensor_filter newValue) {
  BME280_userSettings.Filter_Coefficient = newValue;
}

void BME280_Control::read_BME280(void) {
  if (BME280_userSettings.Sensor_Mode == Adafruit_BME280::MODE_FORCED) {
    bme280_instance->takeForcedMeasurement();
  }
  
  BME280_dataStorage.temperature = bme280_instance->readTemperature();         // Read temperature data in degree Celsius
  BME280_dataStorage.pressure    = bme280_instance->readPressure() / 100.0F;   // Read barometric data in hPa
  BME280_dataStorage.humidity    = bme280_instance->readHumidity();            // Read relative humidity data in %RH
}

float BME280_Control::get_Temperature(void) {
  return BME280_dataStorage.temperature;
}

float BME280_Control::get_Pressure(void) {
  return BME280_dataStorage.pressure;
}

float BME280_Control::get_Humidity(void) {
  return BME280_dataStorage.humidity;
}

void BME280_Control::set_readFlag(void) {
  BME280_readFlag = true;
}

bool BME280_Control::is_readFlag_set(void) {
  return BME280_readFlag;
}

void BME280_Control::clear_readFlag(void) {
  BME280_readFlag = false;
}
