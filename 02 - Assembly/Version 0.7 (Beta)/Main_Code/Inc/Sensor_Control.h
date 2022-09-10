#pragma once

// #include "Sensor_Base.h"

#include "SensorUnits/Anemometer.h"
#include "SensorUnits/Wind_Vane.h"
#include "SensorUnits/Precipitation.h"
#include "SensorUnits/Thermometer_Hygrometer_Barometer.h"


class Sensor_Control {
    public:
        Anemometer_Control *Anemometer_Device;
        WindVane_Control   *WindVane_Device;
        Precipitation      *RainGauge_Device;
        BME280_Control     *BME280_Device;
        DS18B20_Control    *DS18B20_Device;

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
};

extern Sensor_Control General_Sensor_Control;