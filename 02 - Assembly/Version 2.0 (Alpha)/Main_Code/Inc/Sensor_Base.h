#pragma once

#include <Arduino.h>

// #include "RTC.h"

class Sensor_Base {
  public:
    Sensor_Base(void);

    void init(void);

    void set_SensorPin(uint32_t SensorPin);
    uint32_t get_SensorPin(void);

    void update_standby(unsigned int standby_period_max_count = 50);
    void increment_standby_count(void);
    unsigned int get_standby_count(void);
    unsigned int get_standby_period(void);

    void update_sensor_data(void);

    void read_sensor_data(float *external_storage);

    // readFlag operations
    void set_readFlag(void);      // Set readFlag
    bool is_readFlag_set(void);   // Return readFlag value
    void clear_readFlag(void);    // Clear readFlag

    // standbyFlag operations
    void set_standbyFlag(void);      // Set samplingFlag
    bool is_standbyFlag_set(void);   // Return samplingFlag value
    void clear_standbyFlag(void);    // Clear samplingFlag

    // newDataReady operations
    void set_newDataReady(void);      // Set newDataReady
    bool is_newDataReady_set(void);   // Return newDataReady value
    void clear_newDataReady(void);    // Clear newDataReady

    // Device-on-standby routine
    void standby_routine(Sensor_Base* Sensor_Instance);

  protected:
    void update_timestamp(void);

  private:
    uint32_t SensorPin;

    unsigned int standby_period_count,        // Elapsed time in milliseconds from the beginning of a standby period
                 standby_period_max_count;    // Duration of a standby period in milliseconds  
    
    bool readFlag,      // Enable to issue an update of sensor data
         standbyFlag,   // Indicates a standby period
         newDataReady;     // Indicates sensor data is available to be accessed externally
};
