#ifndef _THERMOMETER_HYGROMETER_BAROMETER_H_
#define _THERMOMETER_HYGROMETER_BAROMETER_H_

/*****************
 *** Libraries ***
 *****************/

class Sensor_Base;
#include "Sensor_Base.h"

/* BME280 */
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>



/**********************************************
 *** Constants, macros, and private defines ***
 **********************************************/

/* BME280 */
#define SEALEVELPRESSURE_HPA (1013.25)

typedef struct {    // Structure for sensor settings storage
  Adafruit_BME280::sensor_mode Sensor_Mode;     // Sensor mode. Valid values:
                                                //   + Adafruit_BME280::MODE_SLEEP  -> sleep mode
                                                //   + Adafruit_BME280::MODE_NORMAL -> normal mode (cyclic standby and measurement periods)
                                                //   + Adafruit_BME280::MODE_FORCED -> forced mode (one measurement period)
  
  Adafruit_BME280::sensor_sampling Temperature_Oversampling;    // Sensor sampling rate. Valid values:
  Adafruit_BME280::sensor_sampling Pressure_Oversampling;       //   + Adafruit_BME280::SAMPLING_NONE -> Sampling is turned off
  Adafruit_BME280::sensor_sampling Humidity_Oversampling;       //   + Adafruit_BME280::SAMPLING_X1   -> Sampling rate x1
                                                                //   + Adafruit_BME280::SAMPLING_X2   -> Sampling rate x2
                                                                //   + Adafruit_BME280::SAMPLING_X4   -> Sampling rate x4
                                                                //   + Adafruit_BME280::SAMPLING_X8   -> Sampling rate x8
                                                                //   + Adafruit_BME280::SAMPLING_X16  -> Sampling rate x16
  
  Adafruit_BME280::sensor_filter Filter_Coefficient;    // IIR filter coefficient. Valid values:
                                                        //   + Adafruit_BME280::FILTER_OFF   -> IIR filter is turned off
                                                        //   + Adafruit_BME280::FILTER_X2    -> IIR filter coefficient = 2
                                                        //   + Adafruit_BME280::FILTER_X4    -> IIR filter coefficient = 4
                                                        //   + Adafruit_BME280::FILTER_X8    -> IIR filter coefficient = 8
                                                        //   + Adafruit_BME280::FILTER_X16   -> IIR filter coefficient = 16

  Adafruit_BME280::standby_duration Standby_Duration;     // Sensor standby duration for normal mode (t_cycle = t_measure + t_standby). Valid values:
                                                          //   + Adafruit_BME280::STANDBY_MS_0_5    -> 0.5 ms
                                                          //   + Adafruit_BME280::STANDBY_MS_10     -> 10 ms
                                                          //   + Adafruit_BME280::STANDBY_MS_20     -> 20 ms
                                                          //   + Adafruit_BME280::STANDBY_MS_62_5   -> 62.5 ms
                                                          //   + Adafruit_BME280::STANDBY_MS_125    -> 125 ms
                                                          //   + Adafruit_BME280::STANDBY_MS_250    -> 250 ms
                                                          //   + Adafruit_BME280::STANDBY_MS_500    -> 500 ms
                                                          //   + Adafruit_BME280::STANDBY_MS_1000   -> 1000 ms
} BME280_settings;

typedef struct {      // Structure for sensor read-out storage
  float temperature;
  float humidity;
  float pressure;
} BME280_Data;


/* DS18B20 */
#define PARASITE_MODE_TIMER TIM1
enum PRECISION {R_9BIT = 0x1F, R_10BIT = 0x3F, R_11BIT = 0x5F, R_12BIT = 0x7F};   // Valid DS18B20 thermometer resolution values (9-bit, 10-bit, 11-bit, and 12-bit, respectively)
enum ROM_COMMAND {SEARCH_ROM = 0xF0, READ_ROM = 0x33, MATCH_ROM = 0x55,   // Valid ROM commands provided in the datasheet
                  SKIP_ROM = 0xCC, ALARM_SEARCH = 0xEC};
enum FUNCTION_COMMAND {CONVERT_T = 0x44, WRITE_SCRATCHPAD = 0x4E, READ_SCRATCHPAD = 0xBE,     // Valid function commands provided in the datasheet
                       COPY_SCRATCHPAD = 0x48, RECALL_E2 = 0xB8, READ_POWERSUPPLY = 0xb4};                  

enum POWER_SUPPLY_MODE {EXTERNAL_POWER = 0x01, PARASITIC_POWER = 0x00};   // Values for power mode variable
enum CONVERT_T_DELAY {ENABLE_DELAY = 1, DISABLE_DELAY = 0};     // Valid values for flag requesting a delay after issuing a temperature conversion



/*************************
 *** Class definitions ***
 *************************/

/* BME280 */
class BME280_Control : public Sensor_Base {    
  public:
    Adafruit_BME280* bme280_instance;       // Pointer to global device instance

    // Class constructor(s)
    BME280_Control(void);
    BME280_Control(Adafruit_BME280* bme280_instance);

    // Initialisation
    void init(void);
    void init(Adafruit_BME280* bme280_instance);
    void update_external_device_instance(Adafruit_BME280* bme280_instance);

    // Device settings operations
    void update_BME280_settings(unsigned long custom_readInterval = 1000);    // Update device settings  
    void new_SensorMode(Adafruit_BME280::sensor_mode newValue);   // Set (custom) sensor mode
    void new_TemperatureOversampling(Adafruit_BME280::sensor_sampling newValue);    // Set temperature oversampling 
    void new_PressureOversampling(Adafruit_BME280::sensor_sampling newValue);       // Set pressure oversampling
    void new_HumidityOversampling(Adafruit_BME280::sensor_sampling newValue);       // Set humidity oversampling
    void new_FilterCoefficient(Adafruit_BME280::sensor_filter newValue);            // Set IIR filter coefficient
    void new_StandbyDuration(Adafruit_BME280::standby_duration newValue);           // Set standby duration for normal mode

    // Public operations
    void update_sensor_data(void);       // Fetch latest readings from sensor

    // Data-returning methods
    void read_sensor_data(float *external_storage);
    float get_Temperature(void);  // Only return latest ambient temperature reading   
    float get_Pressure(void);     // Only return latest barometric pressure reading
    float get_Humidity(void);     // Only return latest relative humidity reading   

  private:
    BME280_settings BME280_userSettings;    // Stores the latest (user-input) settings for sensor
    BME280_Data BME280_dataStorage;         // Stores the latest sensor readings

    bool Adafruit_BME280_InstanceAvailable;
};

#endif