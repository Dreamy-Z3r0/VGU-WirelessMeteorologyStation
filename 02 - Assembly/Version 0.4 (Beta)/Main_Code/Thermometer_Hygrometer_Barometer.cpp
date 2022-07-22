#include "Thermometer_Hygrometer_Barometer.h"


/**************
 *** BME280 ***
 **************/

void update_BME280_settings(Adafruit_BME280* bme, BME280_settings* updated_settings) {
  bme->setSampling( updated_settings->Power_Mode,                 // Overwrite BME280 power mode
                    updated_settings->Temperature_Oversampling,   // Overwrite BME280 temperature oversampling
                    updated_settings->Pressure_Oversampling,      // Overwrite BME280 pressure oversampling
                    updated_settings->Humidity_Oversampling,      // Overwrite BME280 humidity oversampling
                    updated_settings->Filter_Coefficient          // Overwrite BME280 IIR filter coefficient
                  );
}

void read_BME280(Adafruit_BME280* bme, BME280_settings* settings, BME280_Data* storage) {
  if (settings->Power_Mode == Adafruit_BME280::MODE_FORCED) {
    bme->takeForcedMeasurement();
  }
  
  storage->temperature = bme->readTemperature();         // Read temperature data in degree Celsius
  storage->pressure    = bme->readPressure() / 100.0F;   // Read barometric data in hPa
  storage->humidity    = bme->readHumidity();            // Read relative humidity data in %RH
}


/***************
 *** DS18B20 ***
 ***************/

// Update DS18B20 One-Wire bus
void update_DS18B20_bus(DS18B20_settings* userSettings, uint32_t bus) {
  userSettings->OneWire_bus = bus;
  userSettings->OneWireBus_NumberOfDevices = 1;
}

/*  Feature reserved for future updates
// Update DS18B20 ROM ID
void update_DS18B20_addr(DS18B20_settings* userSettings, uint8_t* addr) {
  for (uint8_t index = 0; index < 8; index += 1) {
    userSettings->addr[index] = addr[index];
  }
}
*/

// Update DS18B20 settings
void update_DS18B20_settings(DS18B20_settings* userSettings, PRECISION thermometerResolution) {
  userSettings->thermometerResolution = thermometerResolution; 

  switch (thermometerResolution) {
    case R_12BIT:
      userSettings->Conversion_delayTime = 750;
      break;
    case R_11BIT:
      userSettings->Conversion_delayTime = 375;
      break;
    case R_10BIT:
      userSettings->Conversion_delayTime = 188;
      break;
    case R_9BIT:
      userSettings->Conversion_delayTime = 94;
      break;
    default:
      break;
  }
}

// Read temperature in degree Celsius from a single DS18B20
void get_DS18B20_Temperature(DS18B20_settings* userSettings, uint8_t* present, float* output) {
  OneWire ds(userSettings->OneWire_bus);
  uint8_t powerMode;
  uint8_t data[9]; 

  // In case of multiple one-wire devices available, ROM command is set to MATCH_ROM to address a specific address (addr) to avoid conflicts during data communication.
  ROM_COMMAND ROMCommand = SKIP_ROM;
  if (userSettings->OneWireBus_NumberOfDevices > 1) ROMCommand = MATCH_ROM;

  // Fetch the existing settings of the DS18B20
  pushCommands_Full(&ds, present, ROMCommand, userSettings->addr, READ_SCRATCHPAD, data, userSettings, 0);
  if (0 == *present) {
    return;
  }

  // Update thermometer resolution to configuration register
  if (userSettings->thermometerResolution != data[4]) {
    data[4] = userSettings->thermometerResolution;
    pushCommands_Full(&ds, present, ROMCommand, userSettings->addr, WRITE_SCRATCHPAD, data+2, userSettings, 0);
    if (0 == *present) {
      return;
    }
  }

  // Read power mode to determine the necessity of a delay command following the conversion request
  pushCommands_Full(&ds, present, ROMCommand, userSettings->addr, READ_POWERSUPPLY, &powerMode, userSettings, 0);
  if (0 == *present) {
    return;
  }

  // Issue a temperature conversion
  pushCommands_Full(&ds, present, ROMCommand, userSettings->addr, CONVERT_T, data, userSettings, (powerMode == EXTERNAL_POWER ? DISABLE_DELAY : ENABLE_DELAY));
  if (0 == *present) {
    return;
  }

  // Verify the updated thermometer resolution and read the temperature conversion result
  pushCommands_Full(&ds, present, ROMCommand, userSettings->addr, READ_SCRATCHPAD, data, userSettings, 5);
  if (data[4] != userSettings->thermometerResolution) *present = 0;   // Errors may have occurred
  if (0 == *present) {
    return;
  }

  // Extract raw data
  int16_t raw = (data[1] << 8) | data[0];
  switch (userSettings->thermometerResolution) {
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
  *output = (float)raw / 16.0;
}

/* Perform a full functional communication cycle with a DS18B20 
 * 
 * device: OneWire objectof a specific bus
 * present: present pulse returned from the bus after a reset
 * ROMCommand: either MATCH_ROM (0x55) or SKIP_ROM (0xCC) to address the specific DS18B20(s) 
 * ROMdata: ROM code of the slave if MATCH_ROM is the ROMCommand, 0 otherwise
 * functionCommand: DS18B20 function command for a specific operation
 * data: data storage if functionCommand is a read or write command
 * userSettings: holds the delay time for temperature conversion in case of a required delay in parasitic power mode
 * option: If functionCommand is READ_SCRATCHPAD (0xBE), option is the number of bytes desired
 *         from the scratchpad. Set option to 0 if the whole scratchpad is to be read.
 *         In case functionCommand is CONVERT_T (0x44), set option value to 1 if the follow-up
 *         delay is desired, or 0 if not.
 */
void pushCommands_Full(OneWire* device, uint8_t* present, 
                       ROM_COMMAND ROMCommand, uint8_t* ROMdata,
                       FUNCTION_COMMAND functionCommand, uint8_t* data, 
                       DS18B20_settings* userSettings, uint8_t option) {
  // Determine power mode of the 1-wire device if temperature conversion is issued and a follow-up delay is requested
  if ((1 == option) && (CONVERT_T == functionCommand)) {  
    pushCommands_Full(device, present, ROMCommand, ROMdata, READ_POWERSUPPLY, data, userSettings, 0);
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
          delay(userSettings->Conversion_delayTime);
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
