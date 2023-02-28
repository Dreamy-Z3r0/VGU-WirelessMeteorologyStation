/******************************************************************
 ******************************************************************
 ***    DS18B20 test for STM32F103CBT6 on Arduino framework     ***
 *** ---------------------------------------------------------- ***
 ***                                                            ***
 *** DS18B20 is a programmable temperature sensor which com-    ***
 *** municates over One-wire protocol. The possible thermometer ***
 *** resolutions are 0.5ºC (9-bit), 0.25ºC (10-bit), 0.125ºC    ***
 *** (11-bit), and 0.0625ºC (12-bit). Any digital pin on the    ***
 *** microcontroller (master) could be used as a 1-wire bus for ***
 *** communications.                                            *** 
 ******************************************************************
 ******************************************************************/


#include <OneWire.h>

enum PRECISION {R_9BIT = 0x1F, R_10BIT = 0x3F, R_11BIT = 0x5F, R_12BIT = 0x7F};
enum ROM_COMMAND {SEARCH_ROM = 0xF0, READ_ROM = 0x33, MATCH_ROM = 0x55,
                  SKIP_ROM = 0xCC, ALARM_SEARCH = 0xEC};
enum FUNCTION_COMMAND {CONVERT_T = 0x44, WRITE_SCRATCHPAD = 0x4E, READ_SCRATCHPAD = 0xBE,
                       COPY_SCRATCHPAD = 0x48, RECALL_E2 = 0xB8, READ_POWERSUPPLY = 0xb4};                  

enum CONVERT_T_DELAY {ENABLE_DELAY = 1, DISABLE_DELAY = 0};

#define OneWireBus PB3
unsigned int thermometerResolution = R_9BIT;
unsigned int delayTime = 94;  // Delay time in ms for 9-bit thermometer

void setup() {
  Serial.begin(9600);
}

void loop() {
  uint8_t present = 0;
  float temperature;

  getTemperature(OneWireBus, 0, &present, &thermometerResolution, &temperature);
  if (1 == present) {
    Serial.print("Thermometer resolution: ");
    switch (thermometerResolution) {
      case R_9BIT: {
        Serial.println("9-bit");
        thermometerResolution = R_10BIT;
        delayTime = 188;
        break;
      }
      case R_10BIT: {
        Serial.println("10-bit");
        thermometerResolution = R_11BIT;
        delayTime = 375;
        break;
      }
      case R_11BIT: {
        Serial.println("11-bit");
        thermometerResolution = R_12BIT;
        delayTime = 750;
        break;
      }
      case R_12BIT: {
        Serial.println("12-bit");
        thermometerResolution = R_9BIT;
        delayTime = 94;
        break;
      }
      default: {
        break;
      }
    }

    Serial.print("Temperature: ");
    Serial.print(temperature, 4);
    Serial.println(" ºC\n");  
  } else {
    Serial.println("An error may have occurred.");
  }

  delay(2000);
}


// Read temperature in degree Celsius from DS18B20
void getTemperature(uint32_t bus, uint8_t* addr, uint8_t* present, unsigned int* dataResolution, float* output) {
  OneWire ds(bus);
  uint8_t data[9]; 

  // Fetch the existing settings of the DS18B20
  pushCommands_Full(&ds, present, SKIP_ROM, addr, READ_SCRATCHPAD, data, 0);
  if (0 == *present) {
    return;
  }

  // Update thermometer resolution to configuration register
  data[4] = *dataResolution;
  pushCommands_Full(&ds, present, SKIP_ROM, addr, WRITE_SCRATCHPAD, data+2, 0);
  if (0 == *present) {
    return;
  }

  // Issue a temperature conversion with a delay
  pushCommands_Full(&ds, present, SKIP_ROM, addr, CONVERT_T, data, ENABLE_DELAY);
  if (0 == *present) {
    return;
  }

  // Verify the updated thermometer resolution and read the temperature conversion result
  pushCommands_Full(&ds, present, SKIP_ROM, addr, READ_SCRATCHPAD, data, 5);
  if (data[4] != *dataResolution) *present = 0;   // Errors may have occurred
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
  *output = (float)raw / 16.0;
}


/*
 * Perform a full functional communication cycle with DS18B20 
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
                       uint8_t ROMCommand, uint8_t* ROMdata,
                       uint8_t functionCommand, uint8_t* data, uint8_t option) {
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
          delay(delayTime);
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
