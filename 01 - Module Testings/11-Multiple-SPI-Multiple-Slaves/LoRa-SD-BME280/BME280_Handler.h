#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// BME280 data storage structure
typedef struct {
  float temperature;
  float humidity;
  float pressure;
} BME280_Data;

void read_BME280(Adafruit_BME280 *bme, BME280_Data* storage, bool* controlFlag);
