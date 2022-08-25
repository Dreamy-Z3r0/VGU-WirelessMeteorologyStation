# HAL libraries in Arduino platform
*(Since the Arduino platform does not currently support DMA for STM32, the use of HAL libraries is considered)*

Test sequence:
1. **Serial_over_USB**: Test the use of Arduino generic `Serial` over USB port instead of the UART interface.
2. **Serial_with_HAL**: Test the compatibility of Arduino generic `Serial` when HAL module is utilised.
3. **ADC_Blocking_Mode**: Test HAL_ADC in blocking mode and determine ADC sampling rate in this mode.
4. **ADC_IT**: Test HAL_ADC in non-blocking mode with interrupt enabled and determine ADC sampling rate in this mode.
5. **ADC_DMA**: Test HAL_ADC in non-blocking mode using DMA and determine ADC sampling rate in this mode.

Test results:

<br/>
<table>
  <thead>
    <tr>
      <th colspan=5>ADC sampling rates by mode<br/>(buffer_size = 50, Code-based sampling rate = 1.5 cycles></th>
    <tr>
    <tr>
      <th colspan=2>Test mode</th>
      <th>Read time</th>
      <th colspan=2>Sampling rate</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td colspan=2>Arduino API: analogRead()</td>
      <td>3.80 ms</td>
      <td>75.96 us/sample</td>
      <td>13.1648 kHz</td>
    </tr>
    <tr>
      <td rowspan=3>HAL</td>
      <td>Blocking mode</td>
      <td>273 us</td>
      <td>5.46 us/sample</td>
      <td>183.15 kHz</td>
    </tr>
    <tr>
      <td>Interrupt-based</td>
      <td>272 us</td>
      <td>5.44 us/sample</td>
      <td>183.82 kHz</td>
    </tr>
    <tr>
      <td>DMA</td>
      <td>64 us</td>
      <td>1.28 us/sample</td>
      <td>781.25 kHz</td>
    </tr>
  </tbody>
</table>

<br/>
<table>
  <thead>
    <tr>
      <th colspan=4>ADC sampling rates in non-blocking mode with DMA (buffer_size = 50)</th>
    </tr>
    <tr>
      <th rowspan=2>Code-based sampling rate<br \>(cycles)</th>
      <th rowspan=2>Read time<br \>(us)</th>
      <th colspan=2>Sampling rate</th>
    </tr>
    <tr>
      <th>(us/sample)</th>
      <th>(kHz)</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td>1.5</td>
      <td>64</td>
      <td>1.28</td>
      <td>781.25</td>
    </tr>
    <tr>
      <td>7.5</td>
      <td>89</td>
      <td>1.78</td>
      <td>561.80</td>
    </tr>
    <tr>
      <td>13.5</td>
      <td>113</td>
      <td>2.26</td>
      <td>442.48</td>
    </tr>
    <tr>
      <td>28.5</td>
      <td>173</td>
      <td>3.46</td>
      <td>289.02</td>
    </tr>
    <tr>
      <td>41.5</td>
      <td>225</td>
      <td>4.5</td>
      <td>222.22</td>
    </tr>
    <tr>
      <td>55.5</td>
      <td>280</td>
      <td>5.6</td>
      <td>178.57</td>
    </tr>
    <tr>
      <td>71.5</td>
      <td>347</td>
      <td>6.94</td>
      <td>144.09</td>
    </tr>
    <tr>
      <td>239.5</td>
      <td>1020</td>
      <td>20.5</td>
      <td>48.78</td>
    </tr>
  </tbody>
</table>
