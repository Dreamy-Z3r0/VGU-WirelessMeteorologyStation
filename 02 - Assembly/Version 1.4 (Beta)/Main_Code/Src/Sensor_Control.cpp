#include "../Inc/Sensor_Control.h"

Sensor_Control::Sensor_Control(void) {

}

Sensor_Control::Sensor_Control( Anemometer_Control *Anemometer_Device, 
                                WindVane_Control   *WindVane_Device,
                                Precipitation      *RainGauge_Device,
                                BME280_Control     *BME280_Device,
                                DS18B20_Control    *DS18B20_Device     ) {
    this->Anemometer_Device = Anemometer_Device;
    this->WindVane_Device = WindVane_Device;
    this->RainGauge_Device = RainGauge_Device;
    this->BME280_Device = BME280_Device;
    this->DS18B20_Device = DS18B20_Device;
}

void Sensor_Control::init(TIM_TypeDef* AnemometerTimer_Instance) {
    Wire.begin();

    Anemometer_Device->init(AnemometerTimer_Instance);
    WindVane_Device->init();
    RainGauge_Device->init();
    // RainGauge.set_DailyAlarm(9, 0, 0);

    delay(2);
    if (!(BME280_Device->bme280_instance->begin())) {
        #ifdef DEBUGGING_OVER_SERIAL
        Serial.println("Problem initializing BME280.");
        #endif
        return;
    }
    delay(20);

    BME280_Device->new_SensorMode(Adafruit_BME280::MODE_FORCED);
    BME280_Device->new_TemperatureOversampling(Adafruit_BME280::SAMPLING_X2);
    BME280_Device->new_PressureOversampling(Adafruit_BME280::SAMPLING_X16);
    BME280_Device->new_HumidityOversampling(Adafruit_BME280::SAMPLING_X1);
    BME280_Device->new_FilterCoefficient(Adafruit_BME280::FILTER_X16);

    BME280_Device->update_BME280_settings();

    DS18B20_Device->update_DS18B20_settings(R_10BIT);

    init_Timer();
}

void Sensor_Control::init_Timer(TIM_TypeDef* Timer_Instance) {
    HardwareTimer* ControlTimer = new HardwareTimer(Timer_Instance);

    ControlTimer->setMode(1, TIMER_DISABLED);    // Use channel 1 of Timer_Instance in output compare mode, no output
    ControlTimer->setOverflow(1000, MICROSEC_FORMAT);   // Timer overflows every 1ms
    ControlTimer->attachInterrupt(1, std::bind(Sensor_Control_TIM_Ovf_Callback, this));  // ISR run whenever timer overflows for channel 1
}

void Sensor_Control::update_Anemometer_Device(Anemometer_Control *Anemometer_Device) {
    this->Anemometer_Device = Anemometer_Device;
}

void Sensor_Control::update_WindVane_Device(WindVane_Control *WindVane_Device) {
    this->WindVane_Device = WindVane_Device;
}

void Sensor_Control::update_RainGauge_Device(Precipitation *RainGauge_Device) {
    this->RainGauge_Device = RainGauge_Device;
}

void Sensor_Control::update_BME280_Device(BME280_Control *BME280_Device) {
    this->BME280_Device = BME280_Device;
}

void Sensor_Control::update_DS18B20_Device(DS18B20_Control *DS18B20_Device) {
    this->DS18B20_Device = DS18B20_Device;
}

void Sensor_Control::Sensor_Control_Main_Routine(void) {

}

void Sensor_Control::Read_From_Anemometer(void) {
    
}


/***********************************************************************
 *** Internal operation(s) for external interrupt service routine(s) ***
 ***********************************************************************/

// Timer counter overflow callback
void Sensor_Control::Timer_Callback(void) {

}


/*********************************************
 *** External interrupt service routine(s) ***
 *********************************************/

// Interrupt service routine when a timer counter overflows
void Sensor_Control_TIM_Ovf_Callback(Sensor_Control* Sensor_Control_Instance) {
  Sensor_Control_Instance->Timer_Callback();
}