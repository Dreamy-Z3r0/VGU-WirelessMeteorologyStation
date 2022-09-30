#include "../Inc/SX1278_LoRa.h"


LoRa_Control::LoRa_Control(void) {

}

void LoRa_Control::set_SPI(SPIClass& spi, uint32_t SPI_Frequency) {
    LoRa.setSPI(spi);

    LoRa_Configurations.LoRa_SPI_Configurations.SPI_Frequency = SPI_Frequency;
    LoRa.setSPIFrequency(LoRa_Configurations.LoRa_SPI_Configurations.SPI_Frequency);
}

void LoRa_Control::set_SPI( uint32_t MOSI_Pin, uint32_t MISO_Pin, 
                            uint32_t SCLK_Pin, uint32_t NCSS_Pin,
                            uint32_t SPI_Frequency ) 
{
    LoRa_Configurations.LoRa_SPI_Configurations.MOSI_Pin = MISO_Pin;
    LoRa_Configurations.LoRa_SPI_Configurations.MISO_Pin = MISO_Pin;
    LoRa_Configurations.LoRa_SPI_Configurations.SCLK_Pin = SCLK_Pin;
    LoRa_Configurations.LoRa_SPI_Configurations.NCSS_Pin = NCSS_Pin;

    LoRa_Configurations.LoRa_SPI_Configurations.SPI_Frequency = SPI_Frequency;

    SPIClass* NewSPI = new SPIClass(LoRa_Configurations.LoRa_SPI_Configurations.MOSI_Pin,
                                    LoRa_Configurations.LoRa_SPI_Configurations.MISO_Pin,
                                    LoRa_Configurations.LoRa_SPI_Configurations.SCLK_Pin,
                                    LoRa_Configurations.LoRa_SPI_Configurations.NCSS_Pin);
    LoRa.setSPI(*NewSPI);
    LoRa.setSPIFrequency(LoRa_Configurations.LoRa_SPI_Configurations.SPI_Frequency);
}

void LoRa_Control::set_SPI( uint32_t MOSI_Pin, uint32_t MISO_Pin, 
                            uint32_t SCLK_Pin, uint32_t SPI_Frequency ) 
{
    LoRa_Configurations.LoRa_SPI_Configurations.MOSI_Pin = MISO_Pin;
    LoRa_Configurations.LoRa_SPI_Configurations.MISO_Pin = MISO_Pin;
    LoRa_Configurations.LoRa_SPI_Configurations.SCLK_Pin = SCLK_Pin;

    LoRa_Configurations.LoRa_SPI_Configurations.SPI_Frequency = SPI_Frequency;

    SPIClass* NewSPI = new SPIClass(LoRa_Configurations.LoRa_SPI_Configurations.MOSI_Pin,
                                    LoRa_Configurations.LoRa_SPI_Configurations.MISO_Pin,
                                    LoRa_Configurations.LoRa_SPI_Configurations.SCLK_Pin);
    LoRa.setSPI(*NewSPI);
    LoRa.setSPIFrequency(LoRa_Configurations.LoRa_SPI_Configurations.SPI_Frequency);
}