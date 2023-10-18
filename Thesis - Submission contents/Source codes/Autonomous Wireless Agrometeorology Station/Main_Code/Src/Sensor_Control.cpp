#include "../Inc/Sensor_Control.h"

Sensor_Control::Sensor_Control(void) {

}

Sensor_Control::Sensor_Control( Anemometer_Control *Anemometer_Device,
                                WindVane_Control   *WindVane_Device,
                                Precipitation      *RainGauge_Device,
                                BME280_Control     *BME280_Device,
                                DS18B20_Control    *DS18B20_Device ) {
    this->BME280_Device = BME280_Device;
    this->DS18B20_Device = DS18B20_Device;
}

void Sensor_Control::init(void) {
    Latest_Sensor_Status.BME280_status     = INITIAL_STATE;
    Latest_Sensor_Status.DS18B20_status    = INITIAL_STATE;
    Latest_Sensor_Status.Anemometer_status = INITIAL_STATE;
    Latest_Sensor_Status.WindVane_status   = INITIAL_STATE;
    Latest_Sensor_Status.RainGauge_status  = INITIAL_STATE;

    Wire.begin();

    delay(2);
    if (!(BME280_Device->bme280_instance->begin(BME280_I2C_ADDR))) {
        #ifdef SERIAL_MONITOR_OUTPUT
        Serial.println("Problem initialising BME280.");
        #endif
        return;
    }
    delay(20);

    BME280_Device->init();

    BME280_Device->new_SensorMode(Adafruit_BME280::MODE_FORCED);
    BME280_Device->new_TemperatureOversampling(Adafruit_BME280::SAMPLING_X1);
    BME280_Device->new_PressureOversampling(Adafruit_BME280::SAMPLING_X1);
    BME280_Device->new_HumidityOversampling(Adafruit_BME280::SAMPLING_X1);
    BME280_Device->new_FilterCoefficient(Adafruit_BME280::FILTER_OFF);

    BME280_Device->update_BME280_settings();

    DS18B20_Device->init();
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

void Sensor_Control::Sensor_Control_Readings_Handler(Data_Type_Request requestType) {
    Sensor_Update_Request(requestType);
    Sensor_Data_Request(requestType);
}

void Sensor_Control::Sensor_Update_Request(Data_Type_Request requestType) {
    switch (requestType) {
        case ONLY_RAINFALL: {
            RainGauge_Device->update_sensor_data();
            break;
        }
        case ALL_SENSORS: {
            Anemometer_Device->update_sensor_data();
            WindVane_Device->update_sensor_data();
            RainGauge_Device->update_sensor_data();
            BME280_Device->update_sensor_data();
            DS18B20_Device->update_sensor_data(&DS18B20_present);
            break;
        }
        default: {      // <- case REGULARLY_UPDATED
            Anemometer_Device->update_sensor_data();
            WindVane_Device->update_sensor_data();
            BME280_Device->update_sensor_data();
            DS18B20_Device->update_sensor_data(&DS18B20_present);   
            break;         
        }
    }
}

void Sensor_Control::Sensor_Data_Request(Data_Type_Request requestType) {
    switch (requestType) {
        case ONLY_RAINFALL: {
            RainGauge_Device->read_sensor_data(&Latest_Sensor_Readings.Rainfall_Amount);
            Latest_Sensor_Status.RainGauge_status = DATA_AVAILABLE;
            break;
        }
        case ALL_SENSORS: {
            Anemometer_Device->read_sensor_data(&Latest_Sensor_Readings.Wind_Data[0]);
            WindVane_Device->read_sensor_data(&Latest_Sensor_Readings.Wind_Data[1]);
            RainGauge_Device->read_sensor_data(&Latest_Sensor_Readings.Rainfall_Amount);
            BME280_Device->read_sensor_data(Latest_Sensor_Readings.BME280_Data);
            DS18B20_Device->read_sensor_data(&Latest_Sensor_Readings.DS18B20_Temperature);

            Latest_Sensor_Status.Anemometer_status = DATA_AVAILABLE;
            Latest_Sensor_Status.WindVane_status = DATA_AVAILABLE;
            Latest_Sensor_Status.RainGauge_status = DATA_AVAILABLE;
            Latest_Sensor_Status.BME280_status = DATA_AVAILABLE;
            Latest_Sensor_Status.DS18B20_status = DATA_AVAILABLE;

            break;
        }
        default: {      // <- case REGULARLY_UPDATED
            Anemometer_Device->read_sensor_data(&Latest_Sensor_Readings.Wind_Data[0]);
            WindVane_Device->read_sensor_data(&Latest_Sensor_Readings.Wind_Data[1]);
            BME280_Device->read_sensor_data(Latest_Sensor_Readings.BME280_Data);
            DS18B20_Device->read_sensor_data(&Latest_Sensor_Readings.DS18B20_Temperature);

            Latest_Sensor_Status.Anemometer_status = DATA_AVAILABLE;
            Latest_Sensor_Status.WindVane_status = DATA_AVAILABLE;
            Latest_Sensor_Status.BME280_status = DATA_AVAILABLE;
            Latest_Sensor_Status.DS18B20_status = DATA_AVAILABLE;

            break;         
        }
    }
}

void Sensor_Control::Sensor_Data_Printout(void) {
    #if defined(RASPBERRY_PI_LOGGING)

    bool newLine = false;

    if (Latest_Sensor_Status.DS18B20_status == DATA_AVAILABLE) {
        PRECISION thermometerResolution = DS18B20_Device->get_thermometerResolution();
        uint8_t floating_point = (uint8_t)(((thermometerResolution >> 4) + 1) / 2);

        Serial.print(Latest_Sensor_Readings.DS18B20_Temperature, floating_point);
        Serial.print(",");

        newLine = true;
    }

    if (Latest_Sensor_Status.BME280_status == DATA_AVAILABLE) {
        // Print temperature
        Serial.printf("%.2f,", Latest_Sensor_Readings.BME280_Data[0]);    

        // Print humidity  
        Serial.printf("%.2f,", Latest_Sensor_Readings.BME280_Data[2]);       

        // Print barometric pressure 
        Serial.printf("%.2f", Latest_Sensor_Readings.BME280_Data[1]); 

        newLine = true;
    }

    if (newLine) Serial.println();

    #endif
}