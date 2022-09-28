# Assembly
### Alpha release - Version 2.0

Change log:
- Move class instance declarations to a separate header file (Macros_and_Defines.h -> Class_Instances.h)
- Add global access for clock and calendar
- Fix some runtime bugs

Features available:
- Time-keeping: *Real-time clock DS3231*
- Sensor group: 
  - Temperature - humidity - barometric pressure: *BME280*
  - Temperature: *DS18B20*
  - Precipitation: *Tipping bucket as rain gauge*
  - Wind monitoring: Wind speed with *Anemometer* and wind direction with *Wind vane*