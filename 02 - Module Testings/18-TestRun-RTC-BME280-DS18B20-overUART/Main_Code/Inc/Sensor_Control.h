#ifndef _SENSOR_CONTROL_H_
#define _SENSOR_CONTROL_H_

class BME280_Control;

#include "Libraries.h"
#include "Macros_and_Defines.h"

typedef struct {
    float BME280_Data[3];   // BME280: Temperature - Pressure - Humidity
    float DS18B20_Temperature;
} Sensor_Readings;

enum Sensor_Data_Status {DATA_AVAILABLE = 1, INITIAL_STATE = 0};

typedef struct {
    Sensor_Data_Status BME280_status;
    Sensor_Data_Status DS18B20_status;
} Sensor_Status;


/***************************************
 *** Sensor_Control class definition ***
 ***************************************/
class Sensor_Control {
    public:
        BME280_Control     *BME280_Device;
        DS18B20_Control    *DS18B20_Device;

        Sensor_Readings Latest_Sensor_Readings;
        Sensor_Status Latest_Sensor_Status;

        Sensor_Control(void);
        Sensor_Control( BME280_Control     *BME280_Device,
                        DS18B20_Control    *DS18B20_Device );

        void init(void);

        void update_BME280_Device(BME280_Control *BME280_Device);
        void update_DS18B20_Device(DS18B20_Control *DS18B20_Device);   

        void Sensor_Control_Readings_Handler(void);
        void Sensor_Data_Printout(void);

    private:
        uint8_t DS18B20_present;

        void Sensor_Update_Request(void);
        void Sensor_Data_Request(void);
};


#endif