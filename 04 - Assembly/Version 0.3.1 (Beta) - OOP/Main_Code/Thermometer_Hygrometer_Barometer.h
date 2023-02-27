/*****************
 *** Libraries ***
 *****************/

/* BME280 */
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>


/* DS18B20 */
#include <OneWire.h>



/**********************************************
 *** Constants, macros, and private defines ***
 **********************************************/

/* BME280 */
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


/* DS18B20 */
enum PRECISION {R_9BIT = 0x1F, R_10BIT = 0x3F, R_11BIT = 0x5F, R_12BIT = 0x7F};
enum ROM_COMMAND {SEARCH_ROM = 0xF0, READ_ROM = 0x33, MATCH_ROM = 0x55,
                  SKIP_ROM = 0xCC, ALARM_SEARCH = 0xEC};
enum FUNCTION_COMMAND {CONVERT_T = 0x44, WRITE_SCRATCHPAD = 0x4E, READ_SCRATCHPAD = 0xBE,
                       COPY_SCRATCHPAD = 0x48, RECALL_E2 = 0xB8, READ_POWERSUPPLY = 0xb4};                  

enum POWER_SUPPLY_MODE {EXTERNAL_POWER = 0x01, PARASITIC_POWER = 0x00};
enum CONVERT_T_DELAY {ENABLE_DELAY = 1, DISABLE_DELAY = 0};


/*************************
 *** Class definitions ***
 *************************/

/* BME280 */
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


/* DS18B20 */
class DS18B20_Control {
  private:
    uint32_t OneWireBus;    // Digital pin as the one-wire bus of the DS18B20 device
    bool sharedBus;
    uint8_t addr[8];        // Device ROM code
    
    PRECISION thermometerResolution;    // Thermometer resolution setting for DS18B20
    unsigned int Conversion_delayTime;  // Delay time for a temperature conversion to complete 
    
    float temperature;      // Temperature output of the last conversion
    bool DS18B20_readFlag;  // Set to enable a temperature conversion

  public:
    DS18B20_Control(uint32_t OneWireBus, PRECISION thermometerResolution = R_12BIT, bool sharedBus = false);   // Constructor

    void update_DS18B20_OneWireBus(uint32_t OneWireBus);  // Change OneWire bus of the DS18B20 device
    void update_DS18B20_addr(uint8_t* addr);  // Update DS18B20 ROM code
    
    void update_DS18B20_settings(PRECISION thermometerResolution);    // Update DS18B20 thermometer resolution and delay settings
    PRECISION get_thermometerResolution(void);    // Return the current thermometer resolution setting

    void convert_Temperature(uint8_t* present);   // Enable a temperature conversion
    float get_Temperature(void);      // Return the latest temperature conversion result

    void set_readFlag(void);
    bool is_readFlag_set(void);
    void clear_readFlag(void);

  protected:
    /* Perform a full functional communication cycle with a DS18B20 
     * 
     * device: OneWire objectof a specific bus
     * present: present pulse returned from the bus after a reset
     * ROMCommand: either MATCH_ROM (0x55) or SKIP_ROM (0xCC) to address the specific DS18B20(s) 
     * ROMdata: ROM code of the slave if MATCH_ROM is the ROMCommand, 0 otherwise
     * functionCommand: DS18B20 function command for a specific operation
     * data: data storage if functionCommand is a read or write command
     * option: If functionCommand is READ_SCRATCHPAD (0xBE), option is the number of bytes desired
     *         from the scratchpad. Set option to 0 if the whole scratchpad is to be read.
     *         In case functionCommand is CONVERT_T (0x44), set option value to 1 if the follow-up
     *         delay is desired, or 0 if not.
     */
    void pushCommands_Full(OneWire* device, uint8_t* present, 
                           ROM_COMMAND ROMCommand, uint8_t* ROMdata,
                           FUNCTION_COMMAND functionCommand, uint8_t* data, 
                           uint8_t option);
};
