#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>


#define SEALEVELPRESSURE_HPA (1013.25)

typedef struct {
  Adafruit_BME280::sensor_mode Sensor_Mode;
  
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


class BME280_Control {
  private:
    Adafruit_BME280* bme280_instance;
    BME280_settings BME280_userSettings;
    BME280_Data BME280_dataStorage;
    bool BME280_readFlag;
    
  public:
    BME280_Control(Adafruit_BME280* bme280_instance);

    void update_BME280_settings(unsigned long custom_readInterval = 1000);
    void new_SensorMode(Adafruit_BME280::sensor_mode newValue);
    void new_TemperatureOversampling(Adafruit_BME280::sensor_sampling newValue);
    void new_PressureOversampling(Adafruit_BME280::sensor_sampling newValue);
    void new_HumidityOversampling(Adafruit_BME280::sensor_sampling newValue);
    void new_FilterCoefficient(Adafruit_BME280::sensor_filter newValue);

    float get_Temperature(void);
    float get_Pressure(void);
    float get_Humidity(void);

    void read_BME280(void);
    void set_readFlag(void);
    bool is_readFlag_set(void);
    void clear_readFlag(void);
};
