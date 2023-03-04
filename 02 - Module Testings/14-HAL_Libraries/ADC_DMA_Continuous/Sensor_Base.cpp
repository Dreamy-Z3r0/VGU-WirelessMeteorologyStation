#include "Sensor_Base.h"


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
//  RTC_data[0] = RTC_DS3231.readDay();
//  RTC_data[1] = RTC_DS3231.readMonth();
//  RTC_data[2] = RTC_DS3231.readYear();
//
//  RTC_data[3] = RTC_DS3231.readHour();
//  RTC_data[4] = RTC_DS3231.readMinute();
//  RTC_data[5] = RTC_DS3231.readSecond();
}

int Sensor_Base::readDay(void) {
//  return RTC_data[0];
}

int Sensor_Base::readMonth(void) {
//  return RTC_data[1];
}

int Sensor_Base::readYear(void) {
//  return RTC_data[2];
}

int Sensor_Base::readHour(void) {
//  return RTC_data[3];
}

int Sensor_Base::readMinute(void) {
//  return RTC_data[4];
}

int Sensor_Base::readSecond(void) {
//  return RTC_data[5];
}

void Sensor_Base::read_sensor_data(float *external_storage) {
  
}

void Sensor_Base::update_standby(unsigned int standby_period_max_count) {
  this->standby_period_max_count = standby_period_max_count;
}

void Sensor_Base::increment_standby_count(void) {
  this->standby_period_count += 1;
}

unsigned int Sensor_Base::get_standby_count(void) {
  return this->standby_period_count;
}

unsigned int Sensor_Base::get_standby_period(void) {
  return this->standby_period_max_count;
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


/* newDataReady */

// Set newDataReady
void Sensor_Base::set_newDataReady(void) {
  newDataReady = true;
}

// Return newDataReady value
bool Sensor_Base::is_newDataReady_set(void) {
  return newDataReady;
}

// Clear newDataReady
void Sensor_Base::clear_newDataReady(void) {
  newDataReady = false;
}



/*********************************
 *** Device-on-standby routine ***
 *********************************/

void Sensor_Base::standby_routine(Sensor_Base* Sensor_Instance) {
  if (Sensor_Instance->is_standbyFlag_set()) {
    Sensor_Instance->increment_standby_count();

    if (Sensor_Instance->get_standby_period() == Sensor_Instance->get_standby_count()) {
      Sensor_Instance->set_readFlag();
    }
  }
}
