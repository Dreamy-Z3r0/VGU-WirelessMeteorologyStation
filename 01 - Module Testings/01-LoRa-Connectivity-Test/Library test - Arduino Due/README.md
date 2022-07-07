# LoRa connectivity test

- Board: Arduino Due (Programming Port)  
- LoRa module: SX1278 @ 433MHz

[LoRa library](https://github.com/sandeepmistry/arduino-LoRa) is provided by Sandeep Mistry.

Available commands:

|**Function**|**Syntax**|***option***|**Example**|**Note**|
|---|:---:|---|:---:|---|
|Print current LoRa settings on Serial monitor|`settings?`|*(none)*|||
|Change LoRa spreading factor|`sf:`*[option]*|6 / 7 / 8 / 9 / 10 / 11 / 12|`sf:10`||
|Change LoRa signal bandwidth|`sb:`*[option]*|7.8 / 10.4 / 15.6 / 20.8 / 31.25 / 41.7 / 62.5 / 125 / 250 / 500|`sb:62.5`|Signal bandwidth in kHz|
|Change LoRa coding rate|`cr:`*[option]*|5 / 6 / 7 / 8|`cr:8`|The corresponding coding rates are 4/5, 4/6, 4/7, and 4/8|
|Change LoRa sync word|`sw:`*[option]*|00-FF *(not case-sensitive)*|`sw:F4` or `sw:f4`|Not all values work|
