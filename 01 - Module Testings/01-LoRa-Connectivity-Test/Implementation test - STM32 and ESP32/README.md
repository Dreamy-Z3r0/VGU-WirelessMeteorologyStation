# LoRa connectivity test

**Additional URLs for Arduino IDE:**
- ESP32: https://dl.espressif.com/dl/package_esp32_index.json
- STM32: https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json


**Board options:**
- ESP32: NodeMCU-32S
- STM32: Generic STM32F1 series - BluePill F103CB (or C8 with 128k)


**Tools and documentation:**
- [SX1278 datasheet](https://semtech.my.salesforce.com/sfc/p/#E0000000JelG/a/2R0000001Rc1/QnUuV9TviODKUgt_rpBlPz.EZA_PNK7Rpi8HA5..Sbo)
- [LoRa calculator](https://unsigned.io/understanding-lora-parameters/)

[LoRa library](https://github.com/sandeepmistry/arduino-LoRa) is provided by Sandeep Mistry.

The NodeMCU-ESP32 acts as the gateway, and the BluePill F103CB is the LoRa node.

**Connections:**

|**ESP32**|**STM32**|**SX1278**|**Function**|
|:---:|:---:|:---:|:---:|
|GPIO23|PB15|MOSI|SPI MOSI|
|GPIO19|PB14|MISO|SPI MISO|
|GPIO18|PB13|SCK|SPI CLK|
|GPIO5|PB12|NSS|SPI CS|
|||||
|GPIO16 / RX2|PA8|RST|LoRa Reset|
|GPIO17 / TX2|PA11|DIO0|EXTI from LoRa|
