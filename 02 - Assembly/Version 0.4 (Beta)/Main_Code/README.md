# XX-Assembly
### Version 0.4 (beta)

- Initialization of the main code: ***Main_Code.ino***, ***Libraries.h***, and ***Macros_and_Variables.h***
- First feature of the assembly: *Real-time Clock*
- Second feature of the assembly: *Ambient temperature - Relative humidity - Barometric pressure*
- (updated) Second feature of the assembly: Single sensor for only ambient temperature, managed by the formerly released source code ***Thermometer_Hygrometer_Barometer.h*** and ***Thermometer_Hygrometer_Barometer.cpp***
- Third feature of the assembly: *Precipitation*

*Note 1:* The project utilizes only 1 DS18B20 device, thus the **SKIP_ROM** throughout the source code. However, should 2 or more sensors be included in future updates, each device is to be identified and addressed separately during data exchanges by the microcontroller (master).

*Note 2:* Precipitation data only includes rainfall amount in this project.