#ifndef _THERMOMETER_H_
#define _THERMOMETER_H_

/* DS18B20 */
#include <OneWire.h>

#include "Sensor_Base.h"

/* DS18B20 */
#define PARASITE_MODE_TIMER TIM1
enum PRECISION {R_9BIT = 0x1F, R_10BIT = 0x3F, R_11BIT = 0x5F, R_12BIT = 0x7F};   // Valid DS18B20 thermometer resolution values (9-bit, 10-bit, 11-bit, and 12-bit, respectively)
enum ROM_COMMAND {SEARCH_ROM = 0xF0, READ_ROM = 0x33, MATCH_ROM = 0x55,   // Valid ROM commands provided in the datasheet
                  SKIP_ROM = 0xCC, ALARM_SEARCH = 0xEC};
enum FUNCTION_COMMAND {CONVERT_T = 0x44, WRITE_SCRATCHPAD = 0x4E, READ_SCRATCHPAD = 0xBE,     // Valid function commands provided in the datasheet
                       COPY_SCRATCHPAD = 0x48, RECALL_E2 = 0xB8, READ_POWERSUPPLY = 0xb4};                  

enum POWER_SUPPLY_MODE {EXTERNAL_POWER = 0x01, PARASITIC_POWER = 0x00};   // Values for power mode variable
enum CONVERT_T_DELAY {ENABLE_DELAY = 1, DISABLE_DELAY = 0};     // Valid values for flag requesting a delay after issuing a temperature conversion


class DS18B20_Control : public Sensor_Base {
  public:
    // Class constructor(s)
    DS18B20_Control(PRECISION thermometerResolution = R_12BIT, bool sharedBus = false);
    DS18B20_Control(uint32_t SensorPin, PRECISION thermometerResolution = R_12BIT, bool sharedBus = false);   // Constructor

    void init(void);

    void update_DS18B20_addr(uint8_t* addr);  // Update DS18B20 ROM code

    void update_DS18B20_settings(PRECISION thermometerResolution);    // Update DS18B20 thermometer resolution and delay settings
    PRECISION get_thermometerResolution(void);    // Return the current thermometer resolution setting

    void update_sensor_data(uint8_t* present);      // Enable a temperature conversion
    void read_sensor_data(float *external_storage);  // Return the latest temperature conversion result

  private:
    bool sharedBus;         // Indicates whether there are other devices on the same One-Wire bus
    uint8_t addr[8];        // Device ROM code
    
    PRECISION thermometerResolution;    // Thermometer resolution setting for DS18B20
    
    TIM_TypeDef* Timer_Instance;
    HardwareTimer* SensorDelayTimer;    // Timer clocking delay time in parasitic power mode

    OneWire* ds;
    uint8_t* powerMode;
    uint8_t data[9];

    ROM_COMMAND ROMCommand;

    bool ongoing_conversion;
    float temperature;      // Temperature output of the last conversion
  
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
                           uint8_t option = 0);
};

#endif
