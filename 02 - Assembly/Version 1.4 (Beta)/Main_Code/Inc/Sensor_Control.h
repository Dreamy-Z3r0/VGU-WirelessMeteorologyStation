#pragma once

#include "Libraries.h"

#ifndef Sensor_Control_TIM_Instance
#define Sensor_Control_TIM_Instance TIM1
#endif

#ifndef Anemometer_TIM_Instance
#define Anemometer_TIM_Instance TIM2
#endif

typedef struct {
    float Wind_Data[2];     // Wind_Data[0] -> wind speed; Wind_Data[1] -> wind direction
    float Rainfall_Amount;
    float BME280_Data[3];   // BME280: Temperature - Pressure - Humidity
    float DS18B20_Temperature;
} Sensor_Readings;


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

        Sensor_Control(void);
        Sensor_Control( Anemometer_Control *Anemometer_Device, 
                        WindVane_Control   *WindVane_Device,
                        Precipitation      *RainGauge_Device,
                        BME280_Control     *BME280_Device,
                        DS18B20_Control    *DS18B20_Device     );

        void init(TIM_TypeDef* AnemometerTimer_Instance = Anemometer_TIM_Instance);

        void update_Anemometer_Device(Anemometer_Control *Anemometer_Device);
        void update_WindVane_Device(WindVane_Control *WindVane_Device);
        void update_RainGauge_Device(Precipitation *RainGauge_Device);
        void update_BME280_Device(BME280_Control *BME280_Device);
        void update_DS18B20_Device(DS18B20_Control *DS18B20_Device);   

        void Sensor_Control_Main_Routine(void);

        // Internal operation(s) for external interrupt service routine(s)
        void Timer_Callback(void);   // Timer counter overflow callback

    private:
        void init_Timer(TIM_TypeDef* Timer_Instance = Sensor_Control_TIM_Instance);

        void Read_From_Anemometer(void);
};


/*********************************************
 *** External interrupt service routine(s) ***
 *********************************************/
void Sensor_Control_TIM_Ovf_Callback(Sensor_Control* Sensor_Control_Instance);    // Interrupt service routine when a timer counter overflows


/*********************************************************
 *** Declaration of externally accessed class instance ***
 *********************************************************/
// extern Sensor_Control General_Sensor_Control;