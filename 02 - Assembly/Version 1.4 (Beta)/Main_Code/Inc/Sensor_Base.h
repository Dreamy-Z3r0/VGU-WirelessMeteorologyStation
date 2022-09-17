#pragma once

#include "RTC.h"

class Sensor_Base : public DS3231_Control {
  public:
    Sensor_Base(void);

    void init(void);

    void set_SensorPin(uint32_t SensorPin);
    uint32_t get_SensorPin(void);

    void update_standby(unsigned long standby_period_max_count = 50);

    void update_sensor_data(void);

    void read_sensor_data(float *external_storage);

    // readFlag operations
    void set_readFlag(void);      // Set readFlag
    bool is_readFlag_set(void);   // Return readFlag value
    void clear_readFlag(void);    // Clear readFlag

    // // samplingFlag operations
    // void set_samplingFlag(void);      // Set samplingFlag
    // bool is_samplingFlag_set(void);   // Return samplingFlag value
    // void clear_samplingFlag(void);    // Clear samplingFlag

    // standbyFlag operations
    void set_standbyFlag(void);      // Set samplingFlag
    bool is_standbyFlag_set(void);   // Return samplingFlag value
    void clear_standbyFlag(void);    // Clear samplingFlag

    // Device-on-standby routine
    void standby_routine(void);

  protected:
    void update_timestamp(void);

  private:
    uint32_t SensorPin;

    bool readFlag,      // Enable to issue an update of sensor data
         samplingFlag,  // Indicates an on-going sampling routine
         standbyFlag;   // Indicates a standby period

    unsigned int standby_period_count,        // Elapsed time in milliseconds from the beginning of a standby period
                 standby_period_max_count;    // Duration of a standby period in milliseconds  
};
