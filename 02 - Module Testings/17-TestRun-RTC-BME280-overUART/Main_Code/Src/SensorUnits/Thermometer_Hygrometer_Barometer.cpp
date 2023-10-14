#include "../../Inc/SensorUnits/Thermometer_Hygrometer_Barometer.h"


/***************************************
 ***************************************
 *** BME280_Control class definition ***
 ***************************************
 ***************************************/

/*********************
 *** Contructor(s) ***
 *********************/

// Empty constructor
BME280_Control::BME280_Control(void) {
  Adafruit_BME280_InstanceAvailable = false;
}

// Accepts the pointer to global device instance
BME280_Control::BME280_Control(Adafruit_BME280* bme280_instance) {
  update_external_device_instance(bme280_instance);
  Adafruit_BME280_InstanceAvailable = true;
}


/**********************
 *** Initialisation ***
 **********************/

// Initial settings for BME280 device
void BME280_Control::init(void) {
  BME280_userSettings.Sensor_Mode = Adafruit_BME280::MODE_SLEEP;
  BME280_userSettings.Temperature_Oversampling = Adafruit_BME280::SAMPLING_X1;
  BME280_userSettings.Pressure_Oversampling = Adafruit_BME280::SAMPLING_X1;
  BME280_userSettings.Humidity_Oversampling = Adafruit_BME280::SAMPLING_X1;
  BME280_userSettings.Filter_Coefficient = Adafruit_BME280::FILTER_OFF;
  BME280_userSettings.Standby_Duration = Adafruit_BME280::STANDBY_MS_0_5;

  update_BME280_settings();

  set_standbyFlag();
  set_readFlag();
}

// Initialize BME280 device settings with update Adafruit_BME280 instance
void BME280_Control::init(Adafruit_BME280* bme280_instance) {
  update_external_device_instance(bme280_instance);
  init();
}

// Update Adafruit_BME280 instance
void BME280_Control::update_external_device_instance(Adafruit_BME280* bme280_instance) {
  this->bme280_instance = bme280_instance;
  Adafruit_BME280_InstanceAvailable = true;
}


/**********************************
 *** Device settings operations ***
 **********************************/

// Update device settings
void BME280_Control::update_BME280_settings(unsigned long custom_readInterval) {
  if (Adafruit_BME280_InstanceAvailable) {  
    bme280_instance->setSampling( 
                      BME280_userSettings.Sensor_Mode,                // Overwrite BME280 power mode
                      BME280_userSettings.Temperature_Oversampling,   // Overwrite BME280 temperature oversampling
                      BME280_userSettings.Pressure_Oversampling,      // Overwrite BME280 pressure oversampling
                      BME280_userSettings.Humidity_Oversampling,      // Overwrite BME280 humidity oversampling
                      BME280_userSettings.Filter_Coefficient,         // Overwrite BME280 IIR filter coefficient
                      BME280_userSettings.Standby_Duration            // Overwrite BME280 standby duration
                     );
  }
}

// Set (custom) sensor mode
void BME280_Control::new_SensorMode(Adafruit_BME280::sensor_mode newValue) {
  BME280_userSettings.Sensor_Mode = newValue;
}

// Set temperature oversampling
void BME280_Control::new_TemperatureOversampling(Adafruit_BME280::sensor_sampling newValue) {
  BME280_userSettings.Temperature_Oversampling = newValue;
}

// Set pressure oversampling
void BME280_Control::new_PressureOversampling(Adafruit_BME280::sensor_sampling newValue) {
  BME280_userSettings.Pressure_Oversampling = newValue;
}

// Set humidity oversampling
void BME280_Control::new_HumidityOversampling(Adafruit_BME280::sensor_sampling newValue) {
  BME280_userSettings.Humidity_Oversampling = newValue;
}

// Set IIR filter coefficient
void BME280_Control::new_FilterCoefficient(Adafruit_BME280::sensor_filter newValue) {
  BME280_userSettings.Filter_Coefficient = newValue;
}

// Set standby duration for normal mode
void BME280_Control::new_StandbyDuration(Adafruit_BME280::standby_duration newValue) {
  BME280_userSettings.Standby_Duration = newValue;         
}


/**************************
 *** Reading operations ***
 **************************/

// Fetch latest readings from sensor
void BME280_Control::update_sensor_data(void) {
  if (is_readFlag_set() && is_standbyFlag_set()) {
    // Clear standbyFlag to avoid unwanted behaviour with standby_routine() -> archived
    clear_standbyFlag();

    // Issue a data conversion to sensor in forced mode
    if (BME280_userSettings.Sensor_Mode == Adafruit_BME280::MODE_FORCED) {
      bme280_instance->takeForcedMeasurement();
    }
  
    // Fetch the latest sensor readings
    BME280_dataStorage.temperature = bme280_instance->readTemperature();         // Read temperature data in degree Celsius
    BME280_dataStorage.pressure    = bme280_instance->readPressure() / 100.0F;   // Read barometric data in hPa
    BME280_dataStorage.humidity    = bme280_instance->readHumidity();            // Read relative humidity data in %RH

    // Reset sketch level standby period
    set_standbyFlag();
  }
}


/***********************************
 *** Data-returning operation(s) ***
 ***********************************/

void BME280_Control::read_sensor_data(float *external_storage) {
  if (is_readFlag_set() && is_standbyFlag_set()) {  // Double-check status flags to avoid error(s)
    *external_storage = get_Temperature();
    *(external_storage + 1) = get_Pressure();
    *(external_storage + 2) = get_Humidity();

    // Clear sensor read request
    clear_readFlag();
  }
}

// Only return latest ambient temperature reading  
float BME280_Control::get_Temperature(void) {
  return BME280_dataStorage.temperature;
}

// Only return latest barometric pressure reading
float BME280_Control::get_Pressure(void) {
  return BME280_dataStorage.pressure;
}

// Only return latest relative humidity reading
float BME280_Control::get_Humidity(void) {
  return BME280_dataStorage.humidity;
}