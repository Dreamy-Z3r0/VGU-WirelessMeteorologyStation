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
    Latest_Sensor_Status.Anemometer_status = INITIAL_STATE;
    Latest_Sensor_Status.WindVane_status   = INITIAL_STATE;
    Latest_Sensor_Status.RainGauge_status  = INITIAL_STATE;
    Latest_Sensor_Status.BME280_status     = INITIAL_STATE;
    Latest_Sensor_Status.DS18B20_status    = INITIAL_STATE;

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

    BME280_Device->init();

    BME280_Device->new_SensorMode(Adafruit_BME280::MODE_FORCED);
    BME280_Device->new_TemperatureOversampling(Adafruit_BME280::SAMPLING_X2);
    BME280_Device->new_PressureOversampling(Adafruit_BME280::SAMPLING_X16);
    BME280_Device->new_HumidityOversampling(Adafruit_BME280::SAMPLING_X1);
    BME280_Device->new_FilterCoefficient(Adafruit_BME280::FILTER_X16);

    BME280_Device->update_BME280_settings();

    DS18B20_Device->init();

    init_Timer();
}

void Sensor_Control::init_Timer(TIM_TypeDef* Timer_Instance) {
    HardwareTimer* ControlTimer = new HardwareTimer(Timer_Instance);

    ControlTimer->setMode(1, TIMER_DISABLED);    // Use channel 1 of Timer_Instance in output compare mode, no output
    ControlTimer->setOverflow(1000, MICROSEC_FORMAT);   // Timer overflows every 1ms
    ControlTimer->attachInterrupt(1, std::bind(Sensor_Control_TIM_Ovf_Callback, this));  // ISR run whenever timer overflows for channel 1

    ControlTimer->setCount(0);
    ControlTimer->resume();
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
    Sensor_Update_Request();
    Sensor_Data_Request();
}

void Sensor_Control::Sensor_Update_Request(void) {
    Anemometer_Device->update_sensor_data();
    WindVane_Device->update_sensor_data();
    BME280_Device->update_sensor_data();
    DS18B20_Device->update_sensor_data(&DS18B20_present);
}

void Sensor_Control::Sensor_Data_Request(void) {
    if (RainGauge_Device->is_DailyAlarm_available()) {
        RainGauge_Device->clear_DailyAlarm();
        Latest_Sensor_Status.RainGauge_status = DATA_AVAILABLE;
        RainGauge_Device->read_sensor_data(&Latest_Sensor_Readings.Rainfall_Amount);
    }

    if (Anemometer_Device->is_dataReady_set()) {
        Latest_Sensor_Status.Anemometer_status = DATA_AVAILABLE;
        Anemometer_Device->read_sensor_data(&Latest_Sensor_Readings.Wind_Data[0]);
        Anemometer_Device->clear_dataReady();
    }

    if (WindVane_Device->is_dataReady_set()) {
        Latest_Sensor_Status.WindVane_status = DATA_AVAILABLE;
        WindVane_Device->read_sensor_data(&Latest_Sensor_Readings.Wind_Data[1]);
        WindVane_Device->clear_dataReady();
    }

    if (RainGauge_Device->is_dataReady_set()) {
        Latest_Sensor_Status.RainGauge_status = DATA_AVAILABLE;
        RainGauge_Device->read_sensor_data(&Latest_Sensor_Readings.Rainfall_Amount);
        RainGauge_Device->clear_dataReady();
    }

    if (BME280_Device->is_dataReady_set()) {
        Latest_Sensor_Status.BME280_status = DATA_AVAILABLE;
        BME280_Device->read_sensor_data(Latest_Sensor_Readings.BME280_Data);
        BME280_Device->clear_dataReady();
    }

    if (DS18B20_Device->is_dataReady_set()) {
        Latest_Sensor_Status.DS18B20_status = DATA_AVAILABLE;
        DS18B20_Device->read_sensor_data(&Latest_Sensor_Readings.DS18B20_Temperature);
        DS18B20_Device->clear_dataReady();
    }
}

void Sensor_Control::Sensor_Data_Printout(void) {
    // #ifdef DEBUGGING_OVER_SERIAL

    bool newLine = false;

    if (Latest_Sensor_Status.RainGauge_status == DATA_AVAILABLE) {
        Serial.printf("  Precipitation (rainfall): %.4f mm\n", Latest_Sensor_Readings.Rainfall_Amount);
        newLine = true;
    }

    if (Latest_Sensor_Status.Anemometer_status == DATA_AVAILABLE) {
        Serial.printf("  Wind speed = %.2f km/h\n", Latest_Sensor_Readings.Wind_Data[0]);      // Print wind speed
        newLine = true;
    }

    if (Latest_Sensor_Status.WindVane_status == DATA_AVAILABLE) {
        Serial.printf("  Wind direction = %.1f°\n", Latest_Sensor_Readings.Wind_Data[1]);     // Print wind direction
        newLine = true;
    }

    if (Latest_Sensor_Status.DS18B20_status == DATA_AVAILABLE) {
        PRECISION thermometerResolution = DS18B20_Device->get_thermometerResolution();
        uint8_t floating_point = (uint8_t)(((thermometerResolution >> 4) + 1) / 2);

        Serial.print("  Ambient temperature = ");
        Serial.print(Latest_Sensor_Readings.DS18B20_Temperature, floating_point);

        switch (thermometerResolution) {
            case R_9BIT:
                Serial.println(" ºC (9-bit DS18B20)");
                break;
            case R_10BIT:
                Serial.println(" ºC (10-bit DS18B20)");
                break;
            case R_11BIT:
                Serial.println(" ºC (11-bit DS18B20)");
                break;
            case R_12BIT:
                Serial.println(" ºC (12-bit DS18B20)");
                break;
            default:
                break;
        }

        newLine = true;
    }

    if (Latest_Sensor_Status.BME280_status == DATA_AVAILABLE) {
        // Print temperature
        Serial.printf("  Ambient temperature = %.2f ºC (BME280)\n", Latest_Sensor_Readings.BME280_Data[0]);    

        // Print humidity  
        Serial.printf("  Relative humidity = %.2f %%RH\n", Latest_Sensor_Readings.BME280_Data[2]);       

        // Print barometric pressure 
        Serial.printf("  Atmospheric pressure = %.2f hPa\n", Latest_Sensor_Readings.BME280_Data[1]); 

        newLine = true;
    }

    if (newLine) Serial.println();

    // #endif
}


/***********************************************************************
 *** Internal operation(s) for external interrupt service routine(s) ***
 ***********************************************************************/

// Timer counter overflow callback
void Sensor_Control::Timer_Callback(void) {
    Anemometer_Device->standby_routine(Anemometer_Device);
    WindVane_Device->standby_routine(WindVane_Device);
    BME280_Device->standby_routine(BME280_Device);
    DS18B20_Device->standby_routine(DS18B20_Device);
}


/*********************************************
 *** External interrupt service routine(s) ***
 *********************************************/

// Interrupt service routine when a timer counter overflows
void Sensor_Control_TIM_Ovf_Callback(Sensor_Control* Sensor_Control_Instance) {
  Sensor_Control_Instance->Timer_Callback();
}