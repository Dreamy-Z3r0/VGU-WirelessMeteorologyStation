#define input_RainGauge PB3
#define RainfallPerTip  0.2794  // mm

unsigned long Daily_Counter = 0;
double Rainfall_Data;

void setup() {
  pinMode(input_RainGauge, INPUT);
  attachInterrupt(digitalPinToInterrupt(input_RainGauge), RainGauge_Tipping_Callback, FALLING);
}

void loop() {
  if (RainGauge_DailyAlarm) {
    unsigned long Lastest_Record = Daily_Counter;
    Daily_Counter = 0;

    Rainfall_Data = RainfallPerTip * Lastest_Record;
    RainGauge_DailyAlarm = false;
  }
}

void RainGauge_Tipping_Callback(void) {
  Daily_Counter += 1;
}
