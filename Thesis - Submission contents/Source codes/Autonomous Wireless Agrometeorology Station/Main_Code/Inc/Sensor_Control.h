#ifndef _SENSOR_CONTROL_H_
#define _SENSOR_CONTROL_H_

class BME280_Control;

#include "Libraries.h"
#include "Macros_and_Defines.h"

typedef struct {
    float Wind_Data[2];     // Wind_Data[0] -> wind speed; Wind_Data[1] -> wind direction
    float Rainfall_Amount;
    float BME280_Data[3];   // BME280: Temperature - Pressure - Humidity
    float DS18B20_Temperature;
} Sensor_Readings;

enum Sensor_Data_Status {DATA_AVAILABLE = 1, INITIAL_STATE = 0};

typedef struct {
    Sensor_Data_Status Anemometer_status;
    Sensor_Data_Status WindVane_status;
    Sensor_Data_Status RainGauge_status;
    Sensor_Data_Status BME280_status;
    Sensor_Data_Status DS18B20_status;
} Sensor_Status;

enum Data_Type_Request { 
    ALL_SENSORS = 0,           // Data request to all included sensors; used in case of mixed alarms
    REGULARLY_UPDATED = 1,     // Data request to temperature sensor, compound sensor, and wind sensors; used in case of minute alarm
    ONLY_RAINFALL = 3          // Data request to rain gauge; used in case of daily alarm
};


/***************************************
 *** Sensor_Control class definition ***
 ***************************************/
class Sensor_Control {
    public:
        Anemometer_Control *Anemometer_Device;
        WindVane_Control   *WindVane_Device;
        Precipitation      *RainGauge_Device;
        BME280_Control     *BME280_Device;
        DS18B20_Control    *DS18B20_Device;

        Sensor_Readings Latest_Sensor_Readings;
        Sensor_Status Latest_Sensor_Status;

        Sensor_Control(void);
        Sensor_Control( Anemometer_Control *Anemometer_Device, 
                        WindVane_Control   *WindVane_Device,
                        Precipitation      *RainGauge_Device,
                        BME280_Control     *BME280_Device,
                        DS18B20_Control    *DS18B20_Device     );

        void init(void);

        void update_Anemometer_Device(Anemometer_Control *Anemometer_Device);
        void update_WindVane_Device(WindVane_Control *WindVane_Device);
        void update_RainGauge_Device(Precipitation *RainGauge_Device);
        void update_BME280_Device(BME280_Control *BME280_Device);
        void update_DS18B20_Device(DS18B20_Control *DS18B20_Device);   

        void Sensor_Control_Readings_Handler(Data_Type_Request requestType = REGULARLY_UPDATED);
        void Sensor_Data_Printout(void);

    private:
        uint8_t DS18B20_present;

        void Sensor_Update_Request(Data_Type_Request requestType);
        void Sensor_Data_Request(Data_Type_Request requestType);
};


#endif