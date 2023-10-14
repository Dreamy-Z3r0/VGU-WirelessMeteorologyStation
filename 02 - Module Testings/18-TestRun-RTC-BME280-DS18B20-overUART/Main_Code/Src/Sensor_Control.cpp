#include "../Inc/Sensor_Control.h"

Sensor_Control::Sensor_Control(void) {

}

Sensor_Control::Sensor_Control( BME280_Control     *BME280_Device,
                                DS18B20_Control    *DS18B20_Device     ) {
    this->BME280_Device = BME280_Device;
    this->DS18B20_Device = DS18B20_Device;
}

void Sensor_Control::init(void) {
    Latest_Sensor_Status.BME280_status     = INITIAL_STATE;
    Latest_Sensor_Status.DS18B20_status    = INITIAL_STATE;

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
    BME280_Device->new_TemperatureOversampling(Adafruit_BME280::SAMPLING_X2);
    BME280_Device->new_PressureOversampling(Adafruit_BME280::SAMPLING_X16);
    BME280_Device->new_HumidityOversampling(Adafruit_BME280::SAMPLING_X1);
    BME280_Device->new_FilterCoefficient(Adafruit_BME280::FILTER_X16);

    BME280_Device->update_BME280_settings();

    DS18B20_Device->init();
}

void Sensor_Control::update_BME280_Device(BME280_Control *BME280_Device) {
    this->BME280_Device = BME280_Device;
}

void Sensor_Control::update_DS18B20_Device(DS18B20_Control *DS18B20_Device) {
    this->DS18B20_Device = DS18B20_Device;
}

void Sensor_Control::Sensor_Control_Readings_Handler(void) {
    Sensor_Update_Request();
    Sensor_Data_Request();
}

void Sensor_Control::Sensor_Update_Request(void) {
    BME280_Device->update_sensor_data();
    DS18B20_Device->update_sensor_data(&DS18B20_present);
}

void Sensor_Control::Sensor_Data_Request(void) {
    BME280_Device->read_sensor_data(Latest_Sensor_Readings.BME280_Data);
    Latest_Sensor_Status.BME280_status = DATA_AVAILABLE;

    DS18B20_Device->read_sensor_data(&Latest_Sensor_Readings.DS18B20_Temperature);
    Latest_Sensor_Status.DS18B20_status = DATA_AVAILABLE;
}

void Sensor_Control::Sensor_Data_Printout(void) {
    #if defined(SERIAL_MONITOR_OUTPUT)

    bool newLine = false;

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

    #endif
}