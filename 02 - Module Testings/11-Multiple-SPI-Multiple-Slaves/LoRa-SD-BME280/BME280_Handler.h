#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// BME280 settings storage structure
typedef struct {
  Adafruit_BME280::sensor_mode Power_Mode;
  
  Adafruit_BME280::sensor_sampling Temperature_Oversampling;
  Adafruit_BME280::sensor_sampling Pressure_Oversampling;
  Adafruit_BME280::sensor_sampling Humidity_Oversampling;
  
  Adafruit_BME280::sensor_filter Filter_Coefficient;

  unsigned long readInterval;
} BME280_settings;

// BME280 data storage structure
typedef struct {
  float temperature;
  float humidity;
  float pressure;
} BME280_Data;

extern BME280_settings BME280_userSettings;  // Create a local holder for sensor settings
extern BME280_Data BME280_dataStorage;       // BME280 data storage instance

void update_BME280_settings(Adafruit_BME280* bme);
void read_BME280(Adafruit_BME280 *bme, bool* controlFlag);
