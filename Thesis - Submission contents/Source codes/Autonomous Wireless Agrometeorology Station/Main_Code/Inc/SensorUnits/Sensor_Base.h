#ifndef _SENSOR_BASE_H_
#define _SENSOR_BASE_H_

#include <Arduino.h>

class Sensor_Base {
  public:
    Sensor_Base(void);

    void init(void);

    void set_SensorPin(uint32_t SensorPin);
    uint32_t get_SensorPin(void);

    void update_sensor_data(void);

    void read_sensor_data(float *external_storage);

    /* Deprecated; for potential developments
    // Return timestamp: date data
    int readDay(void);    // Day of month
    int readMonth(void);  // Month
    int readYear(void);   // Year

    // Return timestamp: time data
    int readHour(void);     // Hour
    int readMinute(void);   // Minute
    int readSecond(void);   // Second
    */

    // readFlag operations
    void set_readFlag(void);      // Set readFlag
    bool is_readFlag_set(void);   // Return readFlag value
    void clear_readFlag(void);    // Clear readFlag

    // standbyFlag operations
    void set_standbyFlag(void);      // Set samplingFlag
    bool is_standbyFlag_set(void);   // Return samplingFlag value
    void clear_standbyFlag(void);    // Clear samplingFlag

  /* Deprecated; for potential developments
  protected:
    int RTC_data[6];    // Date and time data holder: day - month - year - hour - minute - second
    void update_timestamp(void);
  */

  private:
    uint32_t SensorPin;

    bool readFlag,        // Enable to issue an update of sensor data
         standbyFlag;     // Indicates a standby period
};

#endif