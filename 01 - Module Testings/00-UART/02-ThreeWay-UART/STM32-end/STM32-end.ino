#if !defined(STM32_CORE_VERSION) || (STM32_CORE_VERSION  < 0x01090000)
#error "Due to API change, this sketch is compatible with STM32_CORE_VERSION  >= 0x01090000"
#endif

int count = 0;

void setup() {
  #if defined(TIM1)
  TIM_TypeDef *Instance = TIM1;
#else
  TIM_TypeDef *Instance = TIM2;
#endif

  Serial.begin(9600);
  Serial.setTimeout(100);

  HardwareTimer *MsgTim = new HardwareTimer(Instance);
  MsgTim->setOverflow(1, HERTZ_FORMAT);
  MsgTim->attachInterrupt(MessageRoutine);
  MsgTim->resume();
}

void loop() {
  if (Serial.available()) {
    String rxMessage = Serial.readString();
    ReplyRoutine(rxMessage);
  }
}

void ReplyRoutine(String message) {
  Serial.print("Message received: ");
  Serial.println(message);
}

void MessageRoutine(void) {
  Serial.printf("count = %d\n", count);
  
  if (10 == count) count = 0;
  else count += 1;
}
