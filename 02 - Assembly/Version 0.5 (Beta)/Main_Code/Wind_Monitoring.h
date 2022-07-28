#define numberOfDataPoints 50

#define ADC_resolution 12   // STM32F103 has 12-bit ADCs
#define R 3.3E3

float Vcc = 3.3,      // Initial value for Vcc
      AVref = 3.3;    // Initial value for AVref

#define NumberOfDirection 16
float R_in_given[] = {33.1, 6.57, 8.19, 0.89, 1, 0.69, 2.19, 1.4, 3.88, 3.12, 15.98, 14.1, 119.6, 42.07, 64.9, 21.92};

uint16_t raw_data[numberOfDataPoints];
