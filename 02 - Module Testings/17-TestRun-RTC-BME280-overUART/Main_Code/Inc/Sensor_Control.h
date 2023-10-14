#ifndef _SENSOR_CONTROL_H_
#define _SENSOR_CONTROL_H_

class BME280_Control;

#include "Libraries.h"
#include "Macros_and_Defines.h"


typedef struct {
    float BME280_Data[3];   // BME280: Temperature - Pressure - Humidity
} Sensor_Readings;

enum Sensor_Data_Status {DATA_AVAILABLE = 1, INITIAL_STATE = 0};

typedef struct {
    Sensor_Data_Status BME280_status;
} Sensor_Status;


/***************************************
 *** Sensor_Control class definition ***
 ***************************************/
class Sensor_Control {
    public:
        BME280_Control *BME280_Device;

        Sensor_Readings Latest_Sensor_Readings;
        Sensor_Status Latest_Sensor_Status;

        Sensor_Control(void);
        Sensor_Control(BME280_Control *BME280_Device);

        void init(void);

        void update_BME280_Device(BME280_Control *BME280_Device);

        void Sensor_Control_Readings_Handler(void);
        void Sensor_Data_Printout(void);    // Print the latest readings from all sensors


    private:
        void Sensor_Update_Request(void);   // Request all sensors to update readings
        void Sensor_Data_Request(void);     // Request latest readings from all sensors
        
};


#endif