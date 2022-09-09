#pragma once

#include "Sensor_Base.h"

#include "SensorUnits/Anemometer.h"
#include "SensorUnits/Wind_Vane.h"
#include "SensorUnits/Precipitation.h"
#include "SensorUnits/Thermometer_Hygrometer_Barometer.h"






class Sensor_Control : public Sensor_Base {
    public:
        Sensor_Control(void);

        void init(void);

    private:
        Anemometer_Control Anemometer_Device;
        WindVane_Control WindVane_Device;
        Precipitation RainGauge_Device;
        BME280_Control BME280_Device;
        DS18B20_Control DS18B20_Device;
};

extern Sensor_Control General_Sensor_Control;