# BME280 test for STM32F103CBT6 on Arduino framework
BME280 is a temperature - humidity - barometric pressure sensor which could communicate via either I2C or SPI. This test is designed to test not only the sensor, but also the SPI on the STM32. There are 2 tests as followed:
1. ***BME280 on the default SPI bus (SPI1).*** This test is to determine the interactions between the library and the sensor.
2. ***BME280 on a different SPI bus.*** On the STM32F103CBT6, SPI2 is utilize under the API SPI_2 by the alternative class constructor `SPIClass SPI_2(mosi, miso, sclk, ssel);`. Although *ssel* pin is not compulsory, it is still included in this code. This test determines how stable the communication between STM32 and BME280 is when the default SPI is not used, since the architecture of the aforementioned ARM microcontroller may not be fully compatible with the Arduino platform.

Should other configurations be desired, the followings could be applied:
- Interface:
  - I2C: `Adafruit_BME280 bme;`                                                          
  - Default hardware SPI1: `Adafruit_BME280 bme(BME_CS);`                                                        
  - Software SPI: `Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK);` 
                                     
- Configurations for I2C:                                                       
  - Default: `bme.begin();` -> Also used for SPI
  - Different address: `bme.begin(ADDRESS);`                                                        
  - Different I2C bus: `bme.begin(Address, &Wire_x);` 

**Note:**                                                         
- I2C address could be either *0x77* or *0x76*, thus no more than 2 BME280s on the same I2C bus.
- *Wire_x* is `Wire1` for I2C1, `Wire2` for I2C2, etc.
- **BME_CS** pin for BME280 is user-defined. Any free digital pins could be used as **BME_CS** for SPI, so the maximum allowed number of BME280s on the same SPI bus is the number of free digital pins.  
