# HAL libraries in Arduino platform
*(Since the Arduino platform does not currently support DMA for STM32, the use of HAL libraries is considered)*

Test sequence:
1. **Serial_over_USB**: Test the use of Arduino generic `Serial` over USB port instead of the UART interface.
2. **Serial_with_HAL**: Test the compatibility of Arduino generic `Serial` when HAL module is utilised.
3. **ADC_Blocking_Mode**: Test HAL_ADC in blocking mode and determine ADC sampling rate in this mode.
4. **ADC_IT**: Test HAL_ADC in non-blocking mode with interrupt enabled and determine ADC sampling rate in this mode.
5. **ADC_DMA**: Test HAL_ADC in non-blocking mode using DMA and determine ADC sampling rate in this mode.

Test results:

|Sample rate <br />(sketch level)|Read time|Sampling rate|
|:---:|:---:|:---:|
|1.5 cycles|64 us|1.28 us/sample|
|7.5 cycles|89 us|1.78 us/sample|
|13.5 cycles|113 us|2.26 us/sample|
|28.5 cycles|173 us|3.46 us/sample|
|41.5 cycles|225 us|4.5 us/sample|
|55.5 cycles|280 us|5.6 us/sample|
|71.5 cycles|347 us|6.94 us/sample|
|239.5 cycles|1020 us|20.5 us/sample|