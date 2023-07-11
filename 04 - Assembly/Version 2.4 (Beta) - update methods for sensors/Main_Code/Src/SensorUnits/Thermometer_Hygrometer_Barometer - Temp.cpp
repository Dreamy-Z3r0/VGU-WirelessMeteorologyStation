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
 *** Initialization ***
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
  update_standby(custom_readInterval);

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
    // Clear standbyFlag to avoid unwanted behaviour with standby_routine()
    clear_standbyFlag();

    // Issue a data conversion to sensor in forced mode
    if (BME280_userSettings.Sensor_Mode == Adafruit_BME280::MODE_FORCED) {
      bme280_instance->takeForcedMeasurement();
    }
  
    // Fetch the latest sensor readings
    BME280_dataStorage.temperature = bme280_instance->readTemperature();         // Read temperature data in degree Celsius
    BME280_dataStorage.pressure    = bme280_instance->readPressure() / 100.0F;   // Read barometric data in hPa
    BME280_dataStorage.humidity    = bme280_instance->readHumidity();            // Read relative humidity data in %RH

    // Update timestamp
    update_timestamp();

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



/****************************************
 ****************************************
 *** DS18B20_Control class definition ***
 ****************************************
 ****************************************/

/*********************
 *** Contructor(s) ***
 *********************/

// Accept input pin for One-Wire bus; (optional) thermometer resolution and shared bus indicator
DS18B20_Control::DS18B20_Control(PRECISION thermometerResolution, bool sharedBus) {
  update_DS18B20_settings(thermometerResolution);
  this->sharedBus = sharedBus;

  ongoing_conversion = false;
}

// Accept input pin for One-Wire bus; (optional) thermometer resolution and shared bus indicator
DS18B20_Control::DS18B20_Control(uint32_t SensorPin, PRECISION thermometerResolution, bool sharedBus) {
  set_SensorPin(SensorPin);
  update_DS18B20_settings(thermometerResolution);

  this->sharedBus = sharedBus;
  ongoing_conversion = false;

  Timer_Instance = PARASITE_MODE_TIMER;
}


/**********************
 *** Initialisation ***
 **********************/

// Initial settings for BME280 device
void DS18B20_Control::init(void) {
  update_DS18B20_settings(R_10BIT);
  update_standby(Data_Update_Interval);

  set_standbyFlag();
  set_readFlag();
}


/********************************
 *** Device parameter storage ***
 ********************************/

// Store DS18B20 ROM code
void DS18B20_Control::update_DS18B20_addr(uint8_t* addr) {
  for (uint8_t i = 0; i < 8; i += 1) {
    this->addr[i] = addr[i];
  }
}


/**********************************
 *** Device settings operations ***
 **********************************/

// Update DS18B20 thermometer resolution and delay settings
void DS18B20_Control::update_DS18B20_settings(PRECISION thermometerResolution) {
  this->thermometerResolution = thermometerResolution;
}

// Return the current thermometer resolution setting
PRECISION DS18B20_Control::get_thermometerResolution(void) {
  return thermometerResolution;
}


/**************************
 *** Reading operations ***
 **************************/

// Enable a temperature conversion
void DS18B20_Control::update_sensor_data(uint8_t* present) {
  if (is_readFlag_set() && is_standbyFlag_set()) {
    // Clear standby flag and enter data update routine
    clear_standbyFlag();

    ds = new OneWire(get_SensorPin());

    // In case of multiple one-wire devices available, ROM command is set to MATCH_ROM to address a specific address (addr) to avoid conflicts during data communication.
    ROMCommand = SKIP_ROM;
    if (sharedBus) ROMCommand = MATCH_ROM;

    // Fetch the existing settings of the DS18B20
    pushCommands_Full(ds, present, ROMCommand, addr, READ_SCRATCHPAD, data);
    if (0 == *present) {
      return;
    }

    // Update thermometer resolution to configuration register
    if (thermometerResolution != data[4]) {
      data[4] = thermometerResolution;
      pushCommands_Full(ds, present, ROMCommand, addr, WRITE_SCRATCHPAD, data+2);
      if (0 == *present) {
        return;
      }
    }

    // Read power mode to determine the necessity of a delay command following the conversion request
    powerMode = new uint8_t;
    pushCommands_Full(ds, present, ROMCommand, addr, READ_POWERSUPPLY, powerMode);
    if (0 == *present) {
      return;
    }

    // Issue a temperature conversion
    pushCommands_Full(ds, present, ROMCommand, addr, CONVERT_T, data, (*powerMode == EXTERNAL_POWER ? DISABLE_DELAY : ENABLE_DELAY));
    if (0 == *present) {
      return;
    }
  } else if (is_readFlag_set()) {

  }
}

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
void DS18B20_Control::pushCommands_Full(OneWire* device, uint8_t* present, 
                                        ROM_COMMAND ROMCommand, uint8_t* ROMdata,
                                        FUNCTION_COMMAND functionCommand, uint8_t* data, 
                                        uint8_t option) {
  // Determine power mode of the 1-wire device if temperature conversion is issued and a follow-up delay is requested
  if ((1 == option) && (CONVERT_T == functionCommand)) {  
    pushCommands_Full(device, present, ROMCommand, ROMdata, READ_POWERSUPPLY, data);
  }

  // Perform a reset on the 1-wire bus (step 1 of a communication cycle)
  *present = device->reset();   
  if (0 == *present) return;

  // Write ROM command, then the follow-up operations (if any)
  switch (ROMCommand) { 
    case MATCH_ROM: { // MATCH_ROM (0x55) is issued, followed by the ROM code of a specific slave
      device->write(ROMCommand);
      for (uint8_t index = 0; index < 8; index += 1) {
        device->write(ROMdata[index]);                    
      }
      break;
    }
    case SKIP_ROM: {  // SKIP_ROM (0xCC) is issued
      device->write(ROMCommand);
      break;
    }
    default: {  // Invalid ROM command for a full communication cycle with DS18B20(s)
      *present = 0;
      return;
    }
  }

  // Write function command to the DS18B20 and perform the follow-up operations
  if (CONVERT_T == functionCommand) {
    device->write(functionCommand, option); 
  } else {
    device->write(functionCommand); 
  }
  switch (functionCommand) {  // Follow-up operations of the function command
    case CONVERT_T: {
      ongoing_conversion = true;
      if (1 == option) {  
        if (0 == data[0]) { // If parasitic power mode is in use, a timer clocks the delay period
          SensorDelayTimer = new HardwareTimer(Timer_Instance);

          // Use channel 1 of Timer_Instance in output compare mode, no output
          SensorDelayTimer->setMode(1, TIMER_DISABLED);  

          // Set timer overflow to maximum value
          // Note: Overflow range is [1, 0x10000]
          SensorDelayTimer->setOverflow(65536, MICROSEC_FORMAT);  

          // Attach timer overflow routine
          SensorDelayTimer->attachInterrupt(1, std::bind(Sensor_Control_TIM_Ovf_Callback, this));
          
          // Set timer offset:
          //   + 9-bit resolution: 93.75ms -> 18750us * 5 cycles -> offset = 46786
          //   + 10-bit resolution: 187.5ms -> 62500us * 3 cycles -> offset = 3036
          //   + 11-bit resolution: 375ms -> 62500us * 6 cycles -> offset = 3036
          //   + 12-bit resolution: 750ms -> 62500us * 12 cycles -> offset = 3036
          if (R_9BIT == thermometerResolution) {
            SensorDelayTimer->setCount(46786);
          } else {
            SensorDelayTimer->setCount(3036);
          }

          // Start timer
          SensorDelayTimer->resume();
        } else {
          // Do nothing
        }
      }
      break;
    }
    case WRITE_SCRATCHPAD: {
      for (uint8_t index = 0; index < 3; index += 1) {
        device->write(data[index]); // Write 3 bytes to the scratchpad
      }
      break;
    }
    case READ_SCRATCHPAD: {
      for (uint8_t index = 0; index < (0 == option ? 9 : option); index += 1) {
        data[index] = device->read(); // Read the scratchpad byte-by-byte
      }
      if (0 != option) device->reset();  // Issue a reset when the desired number of bytes is reached
      break;
    }
    case COPY_SCRATCHPAD: {
      delay(10);  // Issue a 10-ms delay and make sure no operations are done on the 1-wire bus
      break;
    }
    case RECALL_E2: {
      while (0 == device->read_bit());  // Wait until the recalling operation is finished
      break;
    }
    case READ_POWERSUPPLY: {
      *data = device->read_bit(); // Read 1-bit power supply mode on the 1-wire bus
      break;
    }
    default: {  // Faulty input of functionCommand, so no device is to be communicated
      *present = 0;
      return;
    }
  }
}


/***********************************
 *** Data-returning operation(s) ***
 ***********************************/

// Return the latest temperature conversion result
void DS18B20_Control::read_sensor_data(float *external_storage) {
  if (is_readFlag_set() && is_standbyFlag_set()) {  // Double-check status flags to avoid error(s)
    *external_storage = temperature;

    // Clear sensor read request
    clear_readFlag();
  }
}