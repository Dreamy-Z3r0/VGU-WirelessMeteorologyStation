# LoRa connectivity test

**Objective(s):** Establish a (fake) LoRa gateway and test the connectivity with a node.

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

**Commands:** The LoRa settings could be changed via Serial.

|**Function**|**Syntax**|***option***|**Example**|**Note**|
|---|:---:|---|:---:|---|
|Print current LoRa settings on Serial monitor|`settings?`|*(none)*||Standalone command|
|Change LoRa spreading factor|`?sf:`*[option]*`_`|6 / 7 / 8 / 9 / 10 / 11 / 12|`sf:10_`||
|Change LoRa signal bandwidth|`?sb:`*[option]*`_`|7.8 / 10.4 / 15.6 / 20.8 / 31.25 / 41.7 / 62.5 / 125 / 250 / 500|`sb:62.5_`|Signal bandwidth in kHz|
|Change LoRa coding rate|`?cr:`*[option]*`_`|5 / 6 / 7 / 8|`cr:8_`|The corresponding coding rates are 4/5, 4/6, 4/7, and 4/8|
|Change LoRa sync word|`?sw:`*[option]*`_`|0x00-0xFF *(not case-sensitive)*|`sw:0xF4_` or `sw:0xf4_`|Not all values work|

***Note 1:*** Multiple commands could be input at once. The commands shall be processed from left to right, starting with the first valid one. Command-processing task shall stop once the last (valid) command is reached and all the previously valid commands shall be used. For example,
- `?sf:7_?sb:10.4_?cr:8_` changes spreading factor, signal bandwidth, and coding rate to 7, 10.4kHz, and 4/8 respectively.
- `?sf:7_?sb:10.4_?cr:8` changes spreading factor and signal bandwidth to 7 and 10.4kHz respectively. Coding rate input is discarded due to wrong input format (missing stop character ***_***).
- `sf:7_?sb:10.4_?cr:8` changes only signal bandwidth to 10.4kHz.
- `settings??sf:7_` is discarded since `settings?` is a standalone command.
- `?sf:7_settings?` changes spreading factor to 7. `settings?` is discarded.

***Note 2:*** In **LoRa.cpp** (source code of LoRa library, version 0.8.0, found within Arduino IDE sketchbook), line 401, change 
    `writeRegister(REG_DIO_MAPPING_1, 0x00);`
to  `writeRegister(REG_DIO_MAPPING_1, 0x01);`
in order to work with preamble detection in the future.
