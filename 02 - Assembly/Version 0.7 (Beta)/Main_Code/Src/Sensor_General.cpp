#include "../Inc/Sensor_General.h"


Sensor_General::Sensor_General(void) {
  
}

void Sensor_General::init(void) {
  
}

void Sensor_General::set_SensorPin(uint32_t SensorPin) {
  this->SensorPin = SensorPin;
}

void Sensor_General::update_sensor_data(void) {
  
}

void Sensor_General::update_timestamp(void) {
  readRTC();
}

void Sensor_General::read_sensor_data(void) {
  
}



/*************************
 *** Flag operation(s) ***
 *************************/

/* readFlag */

// Set readFlag
void Sensor_General::set_readFlag(void) {
  readFlag = true;
}

// Return readFlag value
bool Sensor_General::is_readFlag_set(void) {
  return readFlag;
}

// Clear readFlag
void Sensor_General::clear_readFlag(void) {
  readFlag = false;
}


/* samplingFlag */

// Set samplingFlag
void Sensor_General::set_samplingFlag(void) {
  samplingFlag = true;
}

// Return samplingFlag value
bool Sensor_General::is_samplingFlag_set(void) {
  return samplingFlag;
}

// Clear samplingFlag
void Sensor_General::clear_samplingFlag(void) {
  samplingFlag = false;
}
