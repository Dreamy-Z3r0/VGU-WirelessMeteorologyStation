#ifndef _SX1278_LORA_H_
#define _SX1278_LORA_H_

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

typedef struct {
    String message;

    int packetRssi,
        packetSnr,
        packetFrequencyError;

    bool messageStatus;
} LoRa_Message_Handler;


class LoRa_Control {
    public:
        LoRa_Message_Handler Received_Message;
        LoRa_Message_Handler Outgoing_Message;

        /* Constructor(s) */
        LoRa_Control(void);

        /* Class instance initialization */
        void init(void);

        /* Update SPI settings */
        void set_SPI(SPIClass& spi, uint32_t SPI_Frequency = LORA_DEFAULT_SPI_FREQUENCY);   // Change SPIClass instance and (optional) SPI frequency
        void set_SPI( uint32_t MOSI_Pin, uint32_t MISO_Pin,     // Set SPI for LoRa module by hardware SPI pins and (optional) SPI frequency
                      uint32_t SCLK_Pin, uint32_t NCSS_Pin,
                      uint32_t SPI_Frequency = LORA_DEFAULT_SPI_FREQUENCY);
        void set_SPI( uint32_t MOSI_Pin, uint32_t MISO_Pin, uint32_t SCLK_Pin,  // Set SPI for LoRa module by hardware SPI pins (without NCSS pin) and (optional) SPI frequency
                      uint32_t SPI_Frequency = LORA_DEFAULT_SPI_FREQUENCY);

        /* Update LoRa configurations */
        void set_LoRa_Frequency(long LoRa_Frequency, bool update_LoRa_module = false);  // LoRa frequency; default: 433 MHz
        void set_LoRa_NCSS_Pin(uint32_t Pin);       // NCSS pin for LoRa module SX1278
        void set_LoRa_Reset_Pin(uint32_t Pin);      // Reset pin for LoRa module SX1278
        void set_LoRa_IRQ_Pin(uint32_t Pin);        // IRQ pin for LoRa module SX1278's DIO0

        /* Update LoRa parameters */
        void set_LoRa_SpreadingFactor(int sf);      // Spreading factor (6 .. 12)
        void set_LoRa_SignalBandwidth(long bw);     // Signal bandwidth (7.8 / 10.4 / 15.6 / 20.8 / 31.25 / 41.7 / 62.5 / 125 / 250 / 500 kHz)
        void set_LoRa_CodingRate4(int cr4);         // Denominator for LoRa coding rate (5 .. 8)
        void set_LoRa_SyncWord(int sw);             // Sync word (8-bit integer)
        void set_LoRa_TransmissionPower(int tp);    // Transmission power (-4 to +20 dBm)

        /* Physical layer update */
        void initiate_device(bool forced_initialisation = false);   // Initiate LoRa module SX1278 with an option of forced update
        void push_lora_parameters(void);    // Update LoRa parameters to the LoRa module

    private:
        LoRa_Config LoRa_Configurations;
        int LoRa_Device_Initiated;

        bool new_lora_parameters;
        bool new_sf, new_bw, new_cr, new_sw, new_tp;        
};


/****************************
 *** LoRa event functions ***
 ****************************/

void LoRa_rxMode(void);     // Switch to RX mode
void LoRa_txMode(void);     // Switch to TX mode

void onReceive(int packetSize);     // LoRa on-receive event
void onTxDone(void);                // LoRa post-transmission event

void LoRa_sendMessage(String message);      // Out-going message handler


/***********************************************
 *** LoRa_Control class instance declaration ***
 ***********************************************/
extern LoRa_Control LoRa_Device;

#endif