/****************************
 *** Macros and constants ***
 ****************************/
#define SPI2_MOSI_Pin PB15   // SPI2 MOSI pin
#define SPI2_MISO_Pin PB14   // SPI2 MISO pin
#define SPI2_SCLK_Pin PB13   // SPI2 SCLK pin

#define BME_CS_Pin PA4     // SPI CS pin for BME280
#define SEALEVELPRESSURE_HPA (1013.25)

#define LoRa_frequency 433E6  // LoRa frequency

#define LoRa_CS_Pin    PB12   // SPI NCSS pin for LoRa
#define LoRa_resetPin  PA8    // LoRa reset pin
#define LoRa_irqPin    PA11   // External interrupt pin by LoRa


/**********************
 *** Custom classes ***
 **********************/
SPIClass SPI_2(SPI2_MOSI_Pin, SPI2_MISO_Pin, SPI2_SCLK_Pin);

Adafruit_BME280 bme(BME_CS_Pin, &SPI_2);


/********************
 *** Data holders ***
 ********************/
LoRa_Settings LoRa_settings;      // Local storage for LoRa settings

bool gatewayMessaged = false;

BME280_Data BME280_dataStorage;   // BME280 data storage instance
bool BME280_readFlag = false;     // Read flag to handle requests in non-blocking mode
