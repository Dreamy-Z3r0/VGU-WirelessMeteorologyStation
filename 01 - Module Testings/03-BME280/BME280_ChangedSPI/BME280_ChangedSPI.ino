/******************************************************************
 ******************************************************************
 ***    BME280 test for STM32F103CBT6 on Arduino framework      ***
 *** ---------------------------------------------------------- ***
 ***                                                            ***
 *** BME280 is a temperature - humidity - barometric pressure   ***
 *** sensor which could communicate via either I2C or SPI. This ***
 *** test is designed for the default SPI on the STM32.         ***
 ***                                                            ***
 *** On the STM32F103CBT6, SPI2 is utilize under the API SPI_2  ***
 *** by the alternative class constructor:                      ***
 ***    SPIClass SPI_2(mosi, miso, sclk, ssel);                 ***
 *** Although ssel pin is not compulsory, it is still included  ***   
 *** in this code. CS pin for BME280, however, is user-defined. *** 
 ***                                                            ***
 *** Should other configurations be desired, the followings     ***
 *** could be applied:                                          ***
 ***  - Interface:                                              ***
 ***    + I2C: Adafruit_BME280 bme;                             ***                                                            
 ***    + Default hardware SPI1: Adafruit_BME280 bme(BME_CS);   ***                                                            
 ***    + Software SPI: Adafruit_BME280 bme(BME_CS, BME_MOSI,   ***                                                            
 ***                                        BME_MISO, BME_SCK); ***    
 ***                                                            ***                                        
 *** - Configurations for I2C:                                  ***                                                            
 ***    + Default: bme.begin(); -> Also used for SPI            *** 
 ***    + Different address: bme.begin(ADDRESS);                ***                                                            
 ***    + Different I2C bus: bme.begin(Address, &Wire_x);       ***    
 ***                                                            ***    
 *** Note:                                                      ***                                                            
 ***  - I2C address could be either 0x77 or 0x76, thus no more  ***                                                            
 ***    than 2 BME280s on the same I2C bus.                     ***
 ***  - Wire_x is Wire1 for I2C1, Wire2 for I2C2, etc.          ***
 ***  - Any free digital pins could be used as BME_CS for SPI,  ***                                                        
 ***    so the maximum allowed number of BME280s on the same    ***  
 ***    SPI bus is the number of free digital pins.             ***    
 ******************************************************************
 ******************************************************************/


#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define default_MOSI_Pin PB15
#define default_MISO_Pin PB14
#define default_SCLK_Pin PB13
#define default_SSEL_Pin PB12

#define BME_CS PA4
#define SEALEVELPRESSURE_HPA (1013.25)

SPIClass SPI_2(default_MOSI_Pin, default_MISO_Pin, default_SCLK_Pin, default_SSEL_Pin);
Adafruit_BME280 bme(BME_CS, &SPI_2); // hardware SPI

typedef struct {
  float temperature;
  float humidity;
  float pressure;
} BME280_Data;

BME280_Data BME280_dataStorage;
bool BME280_readFlag = false;

void setup() {
  Serial.begin(9600);

//  SPI.setMOSI(default_MOSI_Pin);
//  SPI.setMISO(default_MISO_Pin);
//  SPI.setSCLK(default_SCLK_Pin);
//  SPI.setSSEL(default_SSEL_Pin);

  unsigned status;
  status = bme.begin();
  if (!status) {
    while (1);
  }

  HardwareTimer *sensorRoutine = new HardwareTimer(TIM4);
  sensorRoutine->setOverflow(1, HERTZ_FORMAT);
  sensorRoutine->attachInterrupt(std::bind(read_BME280, &BME280_dataStorage, &BME280_readFlag));
  sensorRoutine->resume();
}

void loop() {
  if (BME280_readFlag) {
    BME280_readFlag = false;

    Serial.print("Ambient temperature = ");
    Serial.print(BME280_dataStorage.temperature);
    Serial.println(" ºC");

    Serial.print("Relative humidity = ");
    Serial.print(BME280_dataStorage.humidity);
    Serial.println(" %RH");

    Serial.print("Atmospheric pressure = ");
    Serial.print(BME280_dataStorage.pressure);
    Serial.println(" hPa");

    Serial.println();
  }
}

void read_BME280(BME280_Data* storage, bool* controlFlag) {
  storage->temperature = bme.readTemperature();         // Read temperature data in degree Celsius
  storage->pressure    = bme.readPressure() / 100.0F;   // Read barometric data in hPa
  storage->humidity    = bme.readHumidity();            // Read relative humidity data in %RH

  *controlFlag = true;
}
