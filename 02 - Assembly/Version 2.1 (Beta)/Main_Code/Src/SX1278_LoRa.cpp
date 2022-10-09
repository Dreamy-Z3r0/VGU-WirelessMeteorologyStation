#include "../Inc/SX1278_LoRa.h"


/*********************
 *** Contructor(s) ***
 *********************/

LoRa_Control::LoRa_Control(void) {
    LoRa_Device_Initiated = 0;

    new_lora_parameters = false;
    new_sf = false;
    new_bw = false;
    new_cr = false;
    new_sw = false;
    new_tp = false;

    // SPI default parameter(s)
    LoRa_Configurations.LoRa_SPI_Configurations.MOSI_Pin = LoRa_SPI_MOSI_Pin;
    LoRa_Configurations.LoRa_SPI_Configurations.MISO_Pin = LoRa_SPI_MISO_Pin;
    LoRa_Configurations.LoRa_SPI_Configurations.SCLK_Pin = LoRa_SPI_SCLK_Pin;

    LoRa_Configurations.LoRa_SPI_Configurations.SPI_Frequency = LoRa_Module_SPI_Clock;    

    // Initialize message handlers
    Received_Message.message = "";
    Received_Message.packetRssi = 0;
    Received_Message.packetSnr = 0;
    Received_Message.packetFrequencyError = 0;
    Received_Message.messageStatus = false;

    Outgoing_Message.message = "";
    Outgoing_Message.packetRssi = 0;
    Outgoing_Message.packetSnr = 0;
    Outgoing_Message.packetFrequencyError = 0;
    Outgoing_Message.messageStatus = false;
}


/*************************************
 *** Class instance initialization ***
 *************************************/

void LoRa_Control::init(void) {
    // Create an SPIClass instance for LoRa module by hardware SPI pins and configure SPI frequency
    set_SPI(LoRa_SPI_MOSI_Pin, LoRa_SPI_MISO_Pin, LoRa_SPI_SCLK_Pin);
    
    // Set LoRa frequency
    set_LoRa_Frequency(LoRa_Project_Frequency);

    // Set microcontroller's NCSS, Reset, and interrupt pins corresponding to the NSS, RST, and DIO0 pins on LoRa module
    set_LoRa_NCSS_Pin(LoRa_NCSS_Pin);
    set_LoRa_Reset_Pin(LoRa_RST_Pin);
    set_LoRa_IRQ_Pin(LoRa_IRQ_Pin);

    // Set LoRa parameters
    set_LoRa_SpreadingFactor(LoRa_Project_Spreading_Factor);
    set_LoRa_SignalBandwidth(LoRa_Project_Signal_Bandwidth);
    set_LoRa_CodingRate4(LoRa_Project_Coding_Rate_4_Denominator);
    set_LoRa_SyncWord(LoRa_Project_Sync_Word);
    set_LoRa_TransmissionPower(LoRa_Project_Transmission_Power);

    // Force initialisation of LoRa module
    initiate_device(true);
    if (0 == LoRa_Device_Initiated) {   // Break the initialisation in case of failure
        return;
    }

    // Push LoRa parameters from microcontroller to LoRa module
    push_lora_parameters();

    // Set LoRa event functions
    LoRa.onReceive(onReceive);
    LoRa.onTxDone(onTxDone);
}


/***************************
 *** Update SPI settings ***
 ***************************/

// Change SPIClass instance and (optional) SPI frequency
void LoRa_Control::set_SPI(SPIClass& spi, uint32_t SPI_Frequency) {
    LoRa.setSPI(spi);

    LoRa_Configurations.LoRa_SPI_Configurations.SPI_Frequency = SPI_Frequency;
    LoRa.setSPIFrequency(LoRa_Configurations.LoRa_SPI_Configurations.SPI_Frequency);
}

// Set SPI for LoRa module by hardware SPI pins and (optional) SPI frequency
void LoRa_Control::set_SPI( uint32_t MOSI_Pin, uint32_t MISO_Pin, 
                            uint32_t SCLK_Pin, uint32_t NCSS_Pin,
                            uint32_t SPI_Frequency ) 
{
    LoRa_Configurations.LoRa_SPI_Configurations.MOSI_Pin = MOSI_Pin;
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

// Set SPI for LoRa module by hardware SPI pins (without NCSS pin) and (optional) SPI frequency
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


/**********************************
 *** Update LoRa configurations ***
 **********************************/

// LoRa frequency; default: 433 MHz
void LoRa_Control::set_LoRa_Frequency(long LoRa_Frequency, bool update_LoRa_module) {
    long temp = LoRa_Configurations.LoRa_Frequency;
    LoRa_Configurations.LoRa_Frequency = LoRa_Frequency;

    if (update_LoRa_module) {
        if (LoRa_Device_Initiated) {
            initiate_device(true);
        }

        if (0 == LoRa_Device_Initiated) {
            LoRa_Configurations.LoRa_Frequency = temp;

            #ifdef DEBUGGING_OVER_SERIAL
            Serial.printf("Unable to change LoRa frequency.\n");
            #endif
        } else {
            #ifdef DEBUGGING_OVER_SERIAL
            Serial.printf("LoRa frequency changed.\n");
            #endif
        }
    }
}

// NCSS pin for LoRa module SX1278
void LoRa_Control::set_LoRa_NCSS_Pin(uint32_t Pin) {
    LoRa_Configurations.NCSS_Pin = Pin;
    LoRa.setPins(LoRa_Configurations.NCSS_Pin, LoRa_Configurations.Reset_Pin, LoRa_Configurations.IRQ_Pin);
}

// Reset pin for LoRa module SX1278
void LoRa_Control::set_LoRa_Reset_Pin(uint32_t Pin) {
    LoRa_Configurations.Reset_Pin = Pin;
    LoRa.setPins(LoRa_Configurations.NCSS_Pin, LoRa_Configurations.Reset_Pin, LoRa_Configurations.IRQ_Pin);
}

// IRQ pin for LoRa module SX1278's DIO0
void LoRa_Control::set_LoRa_IRQ_Pin(uint32_t Pin) {
    LoRa_Configurations.IRQ_Pin = Pin;
    LoRa.setPins(LoRa_Configurations.NCSS_Pin, LoRa_Configurations.Reset_Pin, LoRa_Configurations.IRQ_Pin);
}


/******************************
 *** Update LoRa parameters ***
 ******************************/

// Spreading factor (6 .. 12)
void LoRa_Control::set_LoRa_SpreadingFactor(int sf) {
    if ((6 > sf) | (12 < sf)) {
        return;
    }

    LoRa_Configurations.LoRa_SpreadingFactor = sf;

    new_lora_parameters = true;
    new_sf = true;
}

// Signal bandwidth (7.8 / 10.4 / 15.6 / 20.8 / 31.25 / 41.7 / 62.5 / 125 / 250 / 500 kHz)
void LoRa_Control::set_LoRa_SignalBandwidth(long bw) {
    if (((long)7.8E3 > bw) | ((long)500E3 < bw)) {
        return;
    }

    LoRa_Configurations.LoRa_SignalBandwidth = bw;

    new_lora_parameters = true;
    new_bw = true;
}

// Denominator for LoRa coding rate (5 .. 8)
void LoRa_Control::set_LoRa_CodingRate4(int cr4) {
    if ((5 > cr4) | (8 < cr4)) {
        return;
    }

    LoRa_Configurations.LoRa_CodingRate4 = cr4;

    new_lora_parameters = true;
    new_cr = true;
}

// Sync word (8-bit integer)
void LoRa_Control::set_LoRa_SyncWord(int sw) {
    if ((0x00 > sw) | (0xFF < sw)) {
        return;
    }

    LoRa_Configurations.LoRa_SyncWord = sw;

    new_lora_parameters = true;
    new_sw = true;
}

// Transmission power (-4 to +20 dBm)
void LoRa_Control::set_LoRa_TransmissionPower(int tp) {
    if ((-4 > tp) | (20 < tp)) {
        return;
    }

    LoRa_Configurations.LoRa_TransmissionPower = tp;

    new_lora_parameters = true;
    new_tp = true;
}


/*****************************
 *** Physical layer update ***
 *****************************/

// Initiate LoRa module SX1278 with an option of forced update
void LoRa_Control::initiate_device(bool forced_initialisation) {
    if (forced_initialisation) {
        LoRa_Device_Initiated = 0;
    }

    if (0 == LoRa_Device_Initiated) {
        uint8_t attempt_no = 0;
        do {
            attempt_no += 1;
            LoRa_Device_Initiated = LoRa.begin(LoRa_Configurations.LoRa_Frequency);

            if (0 == LoRa_Device_Initiated) {
                #ifdef DEBUGGING_OVER_SERIAL
                Serial.printf("LoRa init failed (attempts: %d). Check your connections.\n", attempt_no);
                #endif
                
                if ((10 > attempt_no) & forced_initialisation) {
                    delay(4000);        // Wait for 4s before retrying
                }
            } else {
                #ifdef DEBUGGING_OVER_SERIAL
                Serial.printf("LoRa init failed. Check your connections.\n");
                #endif
            }
        } while ((0 == LoRa_Device_Initiated) & forced_initialisation & (10 >= attempt_no));
    }
}

// Update LoRa parameters to the LoRa module
void LoRa_Control::push_lora_parameters(void) {
    if (new_lora_parameters) {
        if (0 == LoRa_Device_Initiated) {
            return;
        }

        new_lora_parameters = false;

        if (new_sf) {
            new_sf = false;
            LoRa.setSpreadingFactor(LoRa_Configurations.LoRa_SpreadingFactor);
        }

        if (new_bw) {
            new_bw = false;
            LoRa.setSignalBandwidth(LoRa_Configurations.LoRa_SignalBandwidth);
        }

        if (new_cr) {
            new_cr = false;
            LoRa.setCodingRate4(LoRa_Configurations.LoRa_CodingRate4);
        }

        if (new_sw) {
            new_sw = false;
            LoRa.setSyncWord(LoRa_Configurations.LoRa_SyncWord);
        }

        if (new_tp) {
            new_tp = false;
            LoRa.setTxPower(LoRa_Configurations.LoRa_TransmissionPower);
        }
    }
}


/****************************
 *** LoRa event functions ***
 ****************************/

// Switch to RX mode
void LoRa_rxMode(void) {
  LoRa.enableInvertIQ();    // active invert I and Q signals
  LoRa.receive();           // set receive mode
}

// Switch to TX mode
void LoRa_txMode(void) {
  LoRa.idle();              // set standby mode
  LoRa.disableInvertIQ();   // normal mode
}

// LoRa on-receive event
void onReceive(int packetSize) {
    LoRa_Device.Received_Message.message = "";

    while (LoRa.available()) {
        LoRa_Device.Received_Message.message += (char)LoRa.read();
    }

    LoRa_Device.Received_Message.packetRssi = LoRa.packetRssi();
    LoRa_Device.Received_Message.packetSnr  = LoRa.packetSnr();
    LoRa_Device.Received_Message.packetFrequencyError = LoRa.packetFrequencyError();

    #ifdef DEBUGGING_OVER_SERIAL
    Serial.print("Received message: ");
    Serial.println(LoRa_Device.Received_Message.message);

    Serial.printf("RSSI = %d dBm\n", LoRa_Device.Received_Message.packetRssi);
    Serial.printf("SNR = %d dB\n", LoRa_Device.Received_Message.packetSnr);
    Serial.printf("Frequency error = %d Hz\n", LoRa_Device.Received_Message.packetFrequencyError);
    #endif

    LoRa_Device.Received_Message.messageStatus = true;
}

// LoRa post-transmission event
void onTxDone(void) {
    LoRa_rxMode();
}

// Out-going message handler
void LoRa_sendMessage(String message) {
    #ifdef DEBUGGING_OVER_SERIAL
    Serial.print("Outgoing message: ");
    Serial.println(message);
    #endif

    LoRa_txMode();           // set tx mode
    LoRa.beginPacket();      // start packet
    LoRa.print(message);     // add payload
    LoRa.endPacket(true);    // finish packet and send it
}

LoRa_Control LoRa_Device;