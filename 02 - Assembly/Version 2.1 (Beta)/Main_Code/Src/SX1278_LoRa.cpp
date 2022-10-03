#include "../Inc/SX1278_LoRa.h"


LoRa_Control::LoRa_Control(void) {
    LoRa_Device_Initiated = 0;

    // SPI default parameter(s)
    LoRa_Configurations.LoRa_SPI_Configurations.SPI_Frequency = LoRa_Module_SPI_Clock;

    // LoRa interface default parameters
    set_LoRa_Frequency(LoRa_Project_Frequency);

    set_LoRa_NCSS_Pin(LoRa_NCSS_Pin);
    set_LoRa_Reset_Pin(LoRa_RST_Pin);
    set_LoRa_IRQ_Pin(LoRa_IRQ_Pin);

    LoRa_Configurations.LoRa_SpreadingFactor = LoRa_Project_Spreading_Factor;
    LoRa_Configurations.LoRa_SignalBandwidth = LoRa_Project_Signal_Bandwidth;
    LoRa_Configurations.LoRa_CodingRate4 = LoRa_Project_Coding_Rate_4_Denominator;
    LoRa_Configurations.LoRa_SyncWord = LoRa_Project_Sync_Word;
}

void LoRa_Control::init(void) {
    set_SPI();
    
    set_LoRa_Frequency(LoRa_Project_Frequency);

    set_LoRa_NCSS_Pin(LoRa_NCSS_Pin);
    set_LoRa_Reset_Pin(LoRa_RST_Pin);
    set_LoRa_IRQ_Pin(LoRa_IRQ_Pin);

    initiate_device(true);
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

void LoRa_Control::set_LoRa_NCSS_Pin(uint32_t Pin) {
    LoRa_Configurations.NCSS_Pin = Pin;
    LoRa.setPins(LoRa_Configurations.NCSS_Pin, LoRa_Configurations.Reset_Pin, LoRa_Configurations.IRQ_Pin);
}

void LoRa_Control::set_LoRa_Reset_Pin(uint32_t Pin) {
    LoRa_Configurations.Reset_Pin = Pin;
    LoRa.setPins(LoRa_Configurations.NCSS_Pin, LoRa_Configurations.Reset_Pin, LoRa_Configurations.IRQ_Pin);
}

void LoRa_Control::set_LoRa_IRQ_Pin(uint32_t Pin) {
    LoRa_Configurations.IRQ_Pin = Pin;
    LoRa.setPins(LoRa_Configurations.NCSS_Pin, LoRa_Configurations.Reset_Pin, LoRa_Configurations.IRQ_Pin);
}

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

