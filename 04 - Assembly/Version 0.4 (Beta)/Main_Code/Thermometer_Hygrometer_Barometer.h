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


/* DS18B20 */
enum PRECISION {R_9BIT = 0x1F, R_10BIT = 0x3F, R_11BIT = 0x5F, R_12BIT = 0x7F};
enum ROM_COMMAND {SEARCH_ROM = 0xF0, READ_ROM = 0x33, MATCH_ROM = 0x55,
                  SKIP_ROM = 0xCC, ALARM_SEARCH = 0xEC};
enum FUNCTION_COMMAND {CONVERT_T = 0x44, WRITE_SCRATCHPAD = 0x4E, READ_SCRATCHPAD = 0xBE,
                       COPY_SCRATCHPAD = 0x48, RECALL_E2 = 0xB8, READ_POWERSUPPLY = 0xb4};                  

enum POWER_SUPPLY_MODE {EXTERNAL_POWER = 0x01, PARASITIC_POWER = 0x00};
enum CONVERT_T_DELAY {ENABLE_DELAY = 1, DISABLE_DELAY = 0};

typedef struct {
  uint32_t OneWire_bus;
  uint8_t OneWireBus_NumberOfDevices;

  uint8_t addr[8];
  PRECISION thermometerResolution;
  unsigned int Conversion_delayTime;
} DS18B20_settings;


/***************************
 *** Function prototypes ***
 ***************************/

/* BME280 */
void update_BME280_settings(Adafruit_BME280* bme, BME280_settings* updated_settings);
void read_BME280(Adafruit_BME280* bme, BME280_settings* settings, BME280_Data* storage);


/* DS18B20 */
// Update DS18B20 settings
void update_DS18B20_settings(DS18B20_settings* userSettings, PRECISION thermometerResolution);
// Update DS18B20 One-Wire bus
void update_DS18B20_bus(DS18B20_settings* userSettings, uint32_t bus);
/*  Feature reserved for future updates
// Update DS18B20 ROM ID
void update_DS18B20_addr(DS18B20_settings* userSettings, uint8_t* addr);
*/
// Read temperature in degree Celsius from a single DS18B20
void get_DS18B20_Temperature(DS18B20_settings* userSettings, uint8_t* present, float* output);

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
                       DS18B20_settings* userSettings, uint8_t option);
