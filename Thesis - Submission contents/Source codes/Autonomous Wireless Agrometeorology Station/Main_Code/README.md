# Source code for the Autonomous Wireless Agrometeorology Station

_This source code is built using Arduino IDE version 1.8.19_

## Third-party platform package for STM32 microcontroller family
> https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json

## Board setup
+ Board: "Generic STM32F1 series"
+ Board part number: "BluePill F103CB (or C8 with 128k)"
+ U(S)ART support: "Enabled (generic 'Serial')"
+ USB support (if available): "None"
+ USB speed (if available): "Low/Full Speed"
+ Optimize: "Smallest (-Os default)"
+ Debug symbols and core logs: "None"
+ C Runtime Library: "Newlib Nano + Float Printf"
+ Upload method: "STM32CubeProgrammer (SWD)"

## Library dependencies
+ [LoRa](https://github.com/sandeepmistry/arduino-LoRa) *by* **Sandeep Mistry**, version 0.8.0
+ [DS3231](https://github.com/NorthernWidget/DS3231) *by* **Andrew Wickert**, version 1.1.2
+ [Adafruit BME280 Library](https://github.com/adafruit/Adafruit_BME280_Library) *by* **Limor Fried**, version 2.2.2
+ [OneWire](https://github.com/adafruit/Adafruit_BME280_Library) *by* **Paul Stoffregen**, version 2.3.7