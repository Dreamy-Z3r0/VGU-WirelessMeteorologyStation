#pragma once

#include "RTC.h"

class Sensor_General : public DS3231_Control {
  public:
    Sensor_General(void);

    void init(void);

    void set_SensorPin(uint32_t SensorPin);
    uint32_t get_SensorPin(void);

    void update_sensor_data(void);

    void read_sensor_data(float *external_storage);

    // readFlag operations
    void set_readFlag(void);      // Set readFlag
    bool is_readFlag_set(void);   // Return readFlag value
    void clear_readFlag(void);    // Clear readFlag

    // samplingFlag operations
    void set_samplingFlag(void);      // Set samplingFlag
    bool is_samplingFlag_set(void);   // Return samplingFlag value
    void clear_samplingFlag(void);    // Clear samplingFlag

    // standbyFlag operations
    void set_standbyFlag(void);      // Set samplingFlag
    bool is_standbyFlag_set(void);   // Return samplingFlag value
    void clear_standbyFlag(void);    // Clear samplingFlag

  protected:
    void update_timestamp(void);

  private:
    uint32_t SensorPin;

    bool readFlag,      // Enable to issue an update of sensor data
         samplingFlag,  // Indicates an on-going sampling routine
         standbyFlag;   // Indicates a standby period
};
