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
    int  LoRa_SyncWord;
    int  LoRa_TransmissionPower;
} LoRa_Config;


class LoRa_Control {
    public:
        LoRa_Control(void);

        void init(void);

        void set_SPI(SPIClass& spi, uint32_t SPI_Frequency = LORA_DEFAULT_SPI_FREQUENCY);
        void set_SPI( uint32_t MOSI_Pin, uint32_t MISO_Pin, 
                      uint32_t SCLK_Pin, uint32_t NCSS_Pin,
                      uint32_t SPI_Frequency = LORA_DEFAULT_SPI_FREQUENCY);
        void set_SPI( uint32_t MOSI_Pin, uint32_t MISO_Pin, uint32_t SCLK_Pin,
                      uint32_t SPI_Frequency = LORA_DEFAULT_SPI_FREQUENCY);

        void set_LoRa_Frequency(long LoRa_Frequency, bool update_LoRa_module = false);
        void set_LoRa_NCSS_Pin(uint32_t Pin);
        void set_LoRa_Reset_Pin(uint32_t Pin);
        void set_LoRa_IRQ_Pin(uint32_t Pin);

        void set_LoRa_SpreadingFactor(int sf);
        void set_LoRa_SignalBandwidth(long bw);
        void set_LoRa_CodingRate4(int cr4);
        void set_LoRa_SyncWord(int sw);
        void set_LoRa_TransmissionPower(int tp);

        void initiate_device(bool forced_initialisation = false);
        void push_lora_parameters(void);

    private:
        LoRa_Config LoRa_Configurations;
        int LoRa_Device_Initiated;

        bool new_lora_parameters;
        bool new_sf, new_bw, new_cr, new_sw, new_tp;
};