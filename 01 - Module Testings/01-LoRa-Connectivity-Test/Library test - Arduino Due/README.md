# LoRa connectivity test

- Board: Arduino Due (Programming Port)  
- LoRa module: SX1278 @ 433MHz

[LoRa library](https://github.com/sandeepmistry/arduino-LoRa) is provided by Sandeep Mistry.

Available commands: (version 0.2)

|**Function**|**Syntax**|***option***|**Example**|**Note**|
|---|:---:|---|:---:|---|
|Print current LoRa settings on Serial monitor|`settings?`|*(none)*||Standalone command|
|Change LoRa spreading factor|`?sf:`*[option]*`_`|6 / 7 / 8 / 9 / 10 / 11 / 12|`sf:10_`||
|Change LoRa signal bandwidth|`?sb:`*[option]*`_`|7.8 / 10.4 / 15.6 / 20.8 / 31.25 / 41.7 / 62.5 / 125 / 250 / 500|`sb:62.5_`|Signal bandwidth in kHz|
|Change LoRa coding rate|`?cr:`*[option]*`_`|5 / 6 / 7 / 8|`cr:8_`|The corresponding coding rates are 4/5, 4/6, 4/7, and 4/8|
|Change LoRa sync word|`?sw:`*[option]*`_`|0x00-0xFF *(not case-sensitive)*|`sw:0xF4_` or `sw:0xf4_`|Not all values work|

***Note:*** Multiple commands could be input at once. The commands shall be processed from left to right, starting with the first valid one. Command-processing task shall stop once the last (valid) command is reached and all the previously valid commands shall be used. For example,
- `?sf:7_?sb:10.4_?cr:8_` changes spreading factor, signal bandwidth, and coding rate to 7, 10.4kHz, and 4/8 respectively.
- `?sf:7_?sb:10.4_?cr:8` changes spreading factor and signal bandwidth to 7 and 10.4kHz respectively. Coding rate input is discarded due to wrong input format (missing stop character ***_***).
- `sf:7_?sb:10.4_?cr:8` changes only signal bandwidth to 10.4kHz.
- `settings??sf:7_` is discarded since `settings?` is a standalone command.
- `?sf:7_settings?` changes spreading factor to 7. `settings?` is discarded.