#pragma once

#include "Libraries.h"

typedef struct {
    float Wind_Data[2];     // Wind_Data[0] -> wind speed; Wind_Data[1] -> wind direction
    float Rainfall_Amount;
    float BME280_Data[3];
    float DS18B20_Temperature;
} Sensor_Readings;


class Sensor_Control {
    public:
        Anemometer_Control *Anemometer_Device;
        WindVane_Control   *WindVane_Device;
        Precipitation      *RainGauge_Device;
        BME280_Control     *BME280_Device;
        DS18B20_Control    *DS18B20_Device;

        Sensor_Readings Latest_Sensor_Readings;

        Sensor_Control(void);
        Sensor_Control( Anemometer_Control *Anemometer_Device, 
                        WindVane_Control   *WindVane_Device,
                        Precipitation      *RainGauge_Device,
                        BME280_Control     *BME280_Device,
                        DS18B20_Control    *DS18B20_Device );

        void init(void);

        void update_Anemometer_Device(Anemometer_Control *Anemometer_Device);
        void update_WindVane_Device(WindVane_Control *WindVane_Device);
        void update_RainGauge_Device(Precipitation *RainGauge_Device);
        void update_BME280_Device(BME280_Control *BME280_Device);
        void update_DS18B20_Device(DS18B20_Control *DS18B20_Device);   

        void Read_From_Sensors(void);

    private:

};

extern Sensor_Control General_Sensor_Control;