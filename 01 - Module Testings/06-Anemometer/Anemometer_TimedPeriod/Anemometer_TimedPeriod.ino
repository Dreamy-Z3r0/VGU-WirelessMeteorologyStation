/*********************************************************************
 *********************************************************************
 ***    Anemometer test for STM32F103CBT6 on Arduino framework     ***
 *** ------------------------------------------------------------- ***
 ***                                                               ***
 *** An anemometer is used for measuring wind speed. The sensor    ***
 *** is simply a reed switch that closes every time the cups make  ***
 *** a full rotation due to the wind. The methods of reading data  ***
 *** from an anemometer are fairly simple, either to count the     ***
 *** number of rotations during a fixed period (for example, 5s),  ***
 *** or to time 1 single rotation.                                 ***
 ***                                                               ***
 *** Anemometer_TimedPeriod tests the second method. The fixed     ***
 *** duration is pre-defined at the very beginning of the program. ***
 *** Sampling routine is called every minute, and the counter is   ***
 *** incremented whenever the switch (anemometer's reed switch)    ***
 *** toggles, which gives 2 edges per revolution instead of 1. The ***                                                            
 *** wind speed is then calculated as followed:                    ***                                                            
 ***    windSpeed = (inputCounter / (2 * duration)) * (2/3) (m/s)  ***                                                            
 ***              =  inputCounter / (3 * duration)          (m/s)  ***    
 ***                                                               ***                                        
 *** Note 1: According to the datasheet, 1 rotation per second (or ***                                                            
 ***         2 edges per second in this program) is equivalent to  ***
 ***         a wind of 2.4 km/h (2/3 m/s).                         ***                                                  
 ***                                                               ***    
 *** Note 2: Any external interrupt pin could be used as input pin ***                                                            
 ***         for the anemometer.                                   ***                                                               
 *********************************************************************
 *********************************************************************/
 
 
 void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
