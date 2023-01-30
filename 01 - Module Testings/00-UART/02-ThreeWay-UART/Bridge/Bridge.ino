void setup() {
  Serial.begin(9600);
  Serial.setTimeout(100);
}

void loop() {
  if (Serial.available()) {
    String rxMessage = Serial.readString();
    AnalyzeMessage(rxMessage);
  }
}

void AnalyzeMessage(String message) {
  int indicator = message.indexOf(':');
  
  if (-1 == indicator) {
    Serial.print("[bridge] ");
    Serial.print(message);
  } else {
    String tag = message.substring(0, indicator);
    if (tag.equals("forward")) {
      Serial.print(message.substring(indicator+1));
    }
  }
}
