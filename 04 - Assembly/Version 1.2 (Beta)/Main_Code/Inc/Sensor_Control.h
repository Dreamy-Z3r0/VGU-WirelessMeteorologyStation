#pragma once

#include "Libraries.h"

#define Anemometer_TIM_Instance TIM2
#define WindVane_TIM_Insstance  TIM1

typedef struct {
    float Wind_Data[2];     // Wind_Data[0] -> wind speed; Wind_Data[1] -> wind direction
    float Rainfall_Amount;
    float BME280_Data[3];   // BME280: Temperature - Pressure - Humidity
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

        void init(TIM_TypeDef* AnemometerTimer_Instance = Anemometer_TIM_Instance,
                  TIM_TypeDef* WindVaneTimer_Instance = WindVane_TIM_Insstance);

        void update_Anemometer_Device(Anemometer_Control *Anemometer_Device);
        void update_WindVane_Device(WindVane_Control *WindVane_Device);
        void update_RainGauge_Device(Precipitation *RainGauge_Device);
        void update_BME280_Device(BME280_Control *BME280_Device);
        void update_DS18B20_Device(DS18B20_Control *DS18B20_Device);   

        void Read_From_Sensors(void);

    private:
        void Read_From_Anemometer(void);
};

extern Sensor_Control General_Sensor_Control;