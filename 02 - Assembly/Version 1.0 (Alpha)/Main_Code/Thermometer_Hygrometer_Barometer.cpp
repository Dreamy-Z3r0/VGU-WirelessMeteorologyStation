#include "Thermometer_Hygrometer_Barometer.h"


/***************************************
 ***************************************
 *** BME280_Control class definition ***
 ***************************************
 ***************************************/

/*********************
 *** Contructor(s) ***
 *********************/

// Accepts the pointer to global device instance
BME280_Control::BME280_Control(Adafruit_BME280* bme280_instance) {
  this->bme280_instance = bme280_instance;

  BME280_userSettings.Sensor_Mode = Adafruit_BME280::MODE_SLEEP;
  BME280_userSettings.Temperature_Oversampling = Adafruit_BME280::SAMPLING_X1;
  BME280_userSettings.Pressure_Oversampling = Adafruit_BME280::SAMPLING_X1;
  BME280_userSettings.Humidity_Oversampling = Adafruit_BME280::SAMPLING_X1;
  BME280_userSettings.Filter_Coefficient = Adafruit_BME280::FILTER_OFF;
  BME280_userSettings.Standby_Duration = Adafruit_BME280::STANDBY_MS_0_5;

  readFlag = false;
}


/**********************************
 *** Device settings operations ***
 **********************************/

// Update device settings
void BME280_Control::update_BME280_settings(unsigned long custom_readInterval) {
  BME280_userSettings.readInterval = custom_readInterval;
  
  bme280_instance->setSampling( 
                     BME280_userSettings.Sensor_Mode,                // Overwrite BME280 power mode
                     BME280_userSettings.Temperature_Oversampling,   // Overwrite BME280 temperature oversampling
                     BME280_userSettings.Pressure_Oversampling,      // Overwrite BME280 pressure oversampling
                     BME280_userSettings.Humidity_Oversampling,      // Overwrite BME280 humidity oversampling
                     BME280_userSettings.Filter_Coefficient,         // Overwrite BME280 IIR filter coefficient
                     BME280_userSettings.Standby_Duration            // Overwrite BME280 standby duration
                   );
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
void BME280_Control::read_BME280(void) {
  if (BME280_userSettings.Sensor_Mode == Adafruit_BME280::MODE_FORCED) {
    bme280_instance->takeForcedMeasurement();
  }
  
  BME280_dataStorage.temperature = bme280_instance->readTemperature();         // Read temperature data in degree Celsius
  BME280_dataStorage.pressure    = bme280_instance->readPressure() / 100.0F;   // Read barometric data in hPa
  BME280_dataStorage.humidity    = bme280_instance->readHumidity();            // Read relative humidity data in %RH
}


/***********************************
 *** Data-returning operation(s) ***
 ***********************************/

// Return latest ambient temperature reading  
float BME280_Control::get_Temperature(void) {
  return BME280_dataStorage.temperature;
}

// Return latest barometric pressure reading
float BME280_Control::get_Pressure(void) {
  return BME280_dataStorage.pressure;
}

// Return latest relative humidity reading
float BME280_Control::get_Humidity(void) {
  return BME280_dataStorage.humidity;
}


/*****************************
 *** readFlag operation(s) ***
 *****************************/

// Set readFlag
void BME280_Control::set_readFlag(void) {
  readFlag = true;
}

// Return value
bool BME280_Control::is_readFlag_set(void) {
  return readFlag;
}

// Clear readFlag
void BME280_Control::clear_readFlag(void) {
  readFlag = false;
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
DS18B20_Control::DS18B20_Control(uint32_t OneWireBus, PRECISION thermometerResolution, bool sharedBus) {
  update_DS18B20_OneWireBus(OneWireBus);
  update_DS18B20_settings(thermometerResolution);

  this->sharedBus = sharedBus;
}


/********************************
 *** Device parameter storage ***
 ********************************/

// Change OneWire bus of the DS18B20 device
void DS18B20_Control::update_DS18B20_OneWireBus(uint32_t OneWireBus) {
  this->OneWireBus = OneWireBus;
}

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

  switch (thermometerResolution) {
    case R_12BIT:
      Conversion_delayTime = 750;
      break;
    case R_11BIT:
      Conversion_delayTime = 375;
      break;
    case R_10BIT:
      Conversion_delayTime = 188;
      break;
    case R_9BIT:
     Conversion_delayTime = 94;
      break;
    default:
      break;
  }
}

// Return the current thermometer resolution setting
PRECISION DS18B20_Control::get_thermometerResolution(void) {
  return thermometerResolution;
}


/**************************
 *** Reading operations ***
 **************************/

// Enable a temperature conversion
void DS18B20_Control::convert_Temperature(uint8_t* present) {
  OneWire ds(OneWireBus);
  uint8_t powerMode;
  uint8_t data[9];

  // In case of multiple one-wire devices available, ROM command is set to MATCH_ROM to address a specific address (addr) to avoid conflicts during data communication.
  ROM_COMMAND ROMCommand = SKIP_ROM;
  if (sharedBus) ROMCommand = MATCH_ROM;

  // Fetch the existing settings of the DS18B20
  pushCommands_Full(&ds, present, ROMCommand, addr, READ_SCRATCHPAD, data, 0);
  if (0 == *present) {
    return;
  }

  // Update thermometer resolution to configuration register
  if (thermometerResolution != data[4]) {
    data[4] = thermometerResolution;
    pushCommands_Full(&ds, present, ROMCommand, addr, WRITE_SCRATCHPAD, data+2, 0);
    if (0 == *present) {
      return;
    }
  }

  // Read power mode to determine the necessity of a delay command following the conversion request
  pushCommands_Full(&ds, present, ROMCommand, addr, READ_POWERSUPPLY, &powerMode, 0);
  if (0 == *present) {
    return;
  }

  // Issue a temperature conversion
  pushCommands_Full(&ds, present, ROMCommand, addr, CONVERT_T, data, (powerMode == EXTERNAL_POWER ? DISABLE_DELAY : ENABLE_DELAY));
  if (0 == *present) {
    return;
  }

  // Verify the updated thermometer resolution and read the temperature conversion result
  pushCommands_Full(&ds, present, ROMCommand, addr, READ_SCRATCHPAD, data, 5);
  if (data[4] != thermometerResolution) *present = 0;   // Errors may have occurred
  if (0 == *present) {
    return;
  }

  // Extract raw data
  int16_t raw = (data[1] << 8) | data[0];
  switch (thermometerResolution) {
    case R_9BIT: {    // 3 least significant bits are invalid in 9-bit resolution mode
      raw &= ~7;
      break;
    }
    case R_10BIT: {   // 2 least significant bits are invalid in 10-bit resolution mode
      raw &= ~3;
      break;
    }
    case R_11BIT: {   // the least significant bit is invalid in 11-bit resolution mode
      raw &= ~1;
      break;
    }
    default:  // All bits are valid in 12-bit resolution mode
      break;
  }

  // Calculate temperature in degree Celsius
  temperature = (float)raw / 16.0;
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
    pushCommands_Full(device, present, ROMCommand, ROMdata, READ_POWERSUPPLY, data, 0);
  }

  // Perform a reset on the 1-wire bus (step 1 of a communication cycle)
  *present = device->reset();   
  if (0 == *present) return;

  // Writre ROM command, then the follow-up operations (if any)
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
  device->write(functionCommand); 
  switch (functionCommand) {  // Follow-up operations of the function command
    case CONVERT_T: {
      if (1 == option) {  
        if (0 == data[0]) { // Wait by a pre-determined time for temperature conversion if parasitic power mode is in use
          delay(Conversion_delayTime);
        } else {  // Wait until temperature conversion is complete if the device is externally powered
          while (0 == device->read_bit());  
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
float DS18B20_Control::get_Temperature(void) {
  return temperature;
}


/*****************************
 *** readFlag operation(s) ***
 *****************************/

// Set readFlag
void DS18B20_Control::set_readFlag(void) {
  readFlag = true;
}

// Return value
bool DS18B20_Control::is_readFlag_set(void) {
  return readFlag;
}

// Clear readFlag
void DS18B20_Control::clear_readFlag(void) {
  readFlag = false;
}
