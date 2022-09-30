#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

#include "Macros_and_Defines.h"


typedef struct {
    uint32_t MOSI_Pin,      // MOSI pin of the hardware SPI 
             MISO_Pin,      // MISO pin of the hardware SPI
             SCLK_Pin,      // SCLK pin of the hardware SPI
             NCSS_Pin;      // SCLK pin of the hardware SPI (optional)

    // Note: MOSI, MISO, and SCLK pins must be of the same hardware SPI

    uint32_t SPI_Frequency; // Clock frequency for communication via SPI
} SPI_Config;

typedef struct {
    SPI_Config LoRa_SPI_Configurations;
    
    uint32_t NCSS_Pin,
             Reset_Pin,
             IRQ_Pin;

    long LoRa_Frequency;

    int  LoRa_SpreadingFactor;
    long LoRa_SignalBandwidth;
    int  LoRa_CodingRate4;
    long LoRa_PreambleLength;
    int  LoRa_SyncWord;
} LoRa_Config;


class LoRa_Control {
    public:
        LoRa_Control(void);

        void set_SPI(SPIClass& spi, uint32_t SPI_Frequency = LORA_DEFAULT_SPI_FREQUENCY);
        void set_SPI( uint32_t MOSI_Pin, uint32_t MISO_Pin, 
                      uint32_t SCLK_Pin, uint32_t NCSS_Pin,
                      uint32_t SPI_Frequency = LORA_DEFAULT_SPI_FREQUENCY);
        void set_SPI( uint32_t MOSI_Pin, uint32_t MISO_Pin, uint32_t SCLK_Pin,
                      uint32_t SPI_Frequency = LORA_DEFAULT_SPI_FREQUENCY);

        void set_LoRa_Frequency(long LoRa_Frequency = );

    private:
        LoRa_Config LoRa_Configurations;
};