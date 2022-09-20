#include "../Inc/Sensor_Base.h"


Sensor_Base::Sensor_Base(void) {
  
}

void Sensor_Base::init(void) {
  
}

void Sensor_Base::set_SensorPin(uint32_t SensorPin) {
  this->SensorPin = SensorPin;
}

uint32_t Sensor_Base::get_SensorPin(void) {
  return SensorPin; 
}

void Sensor_Base::update_sensor_data(void) {
  
}

void Sensor_Base::update_timestamp(void) {
  // readRTC();
}

void Sensor_Base::read_sensor_data(float *external_storage) {
  
}

void Sensor_Base::update_standby(unsigned long standby_period_max_count) {
  this->standby_period_max_count = standby_period_max_count;
}



/*************************
 *** Flag operation(s) ***
 *************************/

/* readFlag */

// Set readFlag
void Sensor_Base::set_readFlag(void) {
  readFlag = true;
}

// Return readFlag value
bool Sensor_Base::is_readFlag_set(void) {
  return readFlag;
}

// Clear readFlag
void Sensor_Base::clear_readFlag(void) {
  readFlag = false;
}


// /* samplingFlag */

// // Set samplingFlag
// void Sensor_Base::set_samplingFlag(void) {
//   samplingFlag = true;
// }

// // Return samplingFlag value
// bool Sensor_Base::is_samplingFlag_set(void) {
//   return samplingFlag;
// }

// // Clear samplingFlag
// void Sensor_Base::clear_samplingFlag(void) {
//   samplingFlag = false;
// }


/* standbyFlag */

// Set standbyFlag
void Sensor_Base::set_standbyFlag(void) {
  standbyFlag = true;
  standby_period_count = 0;
}

// Return standbyFlag value
bool Sensor_Base::is_standbyFlag_set(void) {
  return standbyFlag;
}

// Clear standbyFlag
void Sensor_Base::clear_standbyFlag(void) {
  standbyFlag = false;
}


/* dataReady */

// Set dataReady
void Sensor_Base::set_dataReady(void) {
  dataReady = true;
}

// Return dataReady value
bool Sensor_Base::is_dataReady_set(void) {
  return dataReady;
}

// Clear dataReady
void Sensor_Base::clear_dataReady(void) {
  dataReady = false;
}



/*********************************
 *** Device-on-standby routine ***
 *********************************/

void Sensor_Base::standby_routine(unsigned int device_standby_period_max_count) {
  if (standbyFlag) {
    standby_period_count += 1;

    if (device_standby_period_max_count == standby_period_count) {
      readFlag = true;
    }
  }
}