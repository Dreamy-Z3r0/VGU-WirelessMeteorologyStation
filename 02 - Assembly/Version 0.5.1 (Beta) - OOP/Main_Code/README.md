# Assembly
### Version 0.5 (beta)
*(Object oriented programming - OOP style of version 0.5)*

- Initialization of the main code: ***Main_Code.ino***, ***Libraries.h***, and ***Macros_and_Variables.h***
- First feature of the assembly: *Real-time Clock*
- Second feature of the assembly: *Ambient temperature - Relative humidity - Barometric pressure*
- Third feature of the assembly: *Precipitation*
- (new) Fourth feature of the assembly: *Wind Monitoring*, starting with wind vane

*Note 1:* The project utilizes only 1 DS18B20 device, thus the **SKIP_ROM** throughout the source code. However, should 2 or more sensors be included in future updates, each device is to be identified and addressed separately during data exchanges by the microcontroller (master), and **sharedBus** must be set `TRUE` for each instance.

*Note 2:* Precipitation data only includes rainfall amount in this project.