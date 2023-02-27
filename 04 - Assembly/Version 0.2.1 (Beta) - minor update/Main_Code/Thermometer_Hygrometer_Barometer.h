#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>


#define SEALEVELPRESSURE_HPA (1013.25)

typedef struct {
  Adafruit_BME280::sensor_mode Power_Mode;
  
  Adafruit_BME280::sensor_sampling Temperature_Oversampling;
  Adafruit_BME280::sensor_sampling Pressure_Oversampling;
  Adafruit_BME280::sensor_sampling Humidity_Oversampling;
  
  Adafruit_BME280::sensor_filter Filter_Coefficient;

  unsigned long readInterval;
} BME280_settings;

typedef struct {
  float temperature;
  float humidity;
  float pressure;
} BME280_Data;


void update_BME280_settings(Adafruit_BME280* bme, BME280_settings* updated_settings);
void read_BME280(Adafruit_BME280* bme, BME280_settings* settings, BME280_Data* storage, bool* controlFlag);
