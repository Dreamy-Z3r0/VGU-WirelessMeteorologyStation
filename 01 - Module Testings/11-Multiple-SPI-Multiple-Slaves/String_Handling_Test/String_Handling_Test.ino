int spreadingFactor;
long signalBandwidth;
int codingRate4;
int syncWord;

bool Input_Message_Handler(String inputString);
bool isValidValue(String stringData, bool* isHEX, bool* isFloat);
int HEX_to_int(uint8_t HEX_value);

void setup() {
  Serial.begin(9600);
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readString();

    Serial.print("input: ");
    Serial.print(input);

    bool validity_check = Input_Message_Handler(input);
    if (validity_check) Serial.println("\nAt least 1 input is available and valid.\n");
    else Serial.println("\nNo valid input.\n");
    Serial.println("");
  }
}

bool Input_Message_Handler(String inputString) {
  Serial.println("\nString processing scope:");
  
  Serial.print("  String to process: ");
  Serial.println(inputString);
  
  int command_start_index = inputString.indexOf('?'),
      command_stop_index = -1,
      colon_index = -1;

  if (0 != command_start_index) {
    if (-1 == command_start_index) {
      Serial.println("  Invalid command format: no start character.");
      return false;
    } else {
      inputString.remove(0, command_start_index);
      command_start_index = 0;
      colon_index = inputString.indexOf(':');
      
      Serial.print("  Command starts at ");
      Serial.println(command_start_index);
    }
  } else {
    Serial.print("  Command starts at ");
    Serial.println(command_start_index);
    colon_index = inputString.indexOf(':');
  }

  if (-1 == colon_index) {
    Serial.println("  Invalid command format: no colon");
    return false;
  } else {
    Serial.print("  Colon at ");
    Serial.println(colon_index);
    command_stop_index = inputString.indexOf(' ');
  }    
   
  if (-1 == command_stop_index) {
    Serial.println("  Invalid command format: no stop character.");
    return false;
  } else {
    Serial.print("  Command stops at ");
    Serial.println(command_stop_index);
  }
  
  String command = inputString.substring(0, colon_index);

  Serial.print("  Input type: ");
  if (command.equals("?sf")) {
    Serial.print("spreading factor\n  Value: ");
    command = inputString.substring(colon_index+1, command_stop_index);
    bool isHEX, isFloat;
    if (isValidValue(command, &isHEX, &isFloat)) {
      if (isHEX || isFloat) {
        Serial.println(" (invalid - float or hex input)");
        return false;
      } else {
        int value = command.toInt();
        if ((6 <= value) && (12 >= value)) {
          spreadingFactor = value;
          Serial.println(spreadingFactor);
        } else {
          Serial.println(" (invalid input range)");
          return false;
        }
      }
    } else {
      Serial.println(" (invalid value)");
      return false;
    }
  } else if (command.equals("?sb")) {
    Serial.print("signal bandwidth\n  Value: ");
    command = inputString.substring(colon_index+1, command_stop_index);
    bool isHEX, isFloat;
    if (isValidValue(command, &isHEX, &isFloat)) {
      if (isHEX) {
        Serial.println(" (invalid - hex input)");
        return false;
      } else {
        long value = (long)(command.toFloat() * 1000);
        switch (value) {
          case (long)7.8E3:
          case (long)10.4E3:
          case (long)15.6E3:
          case (long)20.8E3:
          case (long)31.25E3:
          case (long)41.7E3:
          case (long)62.5E3:
          case (long)125E3:
          case (long)250E3:
          case (long)500E3: { 
            signalBandwidth = value; 
            if (31.25E3 == signalBandwidth)    
              Serial.print("31.25");
            else if (125E3 <= signalBandwidth) 
              Serial.print((int)(signalBandwidth/1E3));
            else 
              Serial.print((float)(signalBandwidth/1E3), 1);
            Serial.println("kHz");
            break;
          }
          default: {
            Serial.println(" (invalid value)");
            return false;
            break;
          }
        }
      }
    } else {
      Serial.println(" (invalid)");
      return false;
    }
  } else if (command.equals("?cr")) {
    Serial.print("coding rate\n  Value: ");
    command = inputString.substring(colon_index+1, command_stop_index);
    bool isHEX, isFloat;
    if (isValidValue(command, &isHEX, &isFloat)) {
      if (isHEX || isFloat) {
        Serial.println(" (invalid - float or hex input)");
        return false;
      } else {
        int value = command.toInt();
        if ((5 <= value) && (8 >= value)) {
          codingRate4 = value;
          Serial.print("4/");
          Serial.println(codingRate4);
        } else {
          Serial.println(" (invalid value)");
          return false;
        }
      }
    } else {
      Serial.println(" (invalid)");
      return false;
    }
  }  else if (command.equals("?sw")) {
    Serial.print("sync word\n  Value: ");
    command = inputString.substring(colon_index+1, command_stop_index);
    bool isHEX, isFloat;
    if (isValidValue(command, &isHEX, &isFloat)) {
      if (!isHEX) {
        Serial.println(" (invalid - not HEX)");
      } else {
        command.toLowerCase();
        uint8_t char0 = command.charAt(2),
                char1 = command.charAt(3);
        
        int value = (HEX_to_int(char0) << 4) | HEX_to_int(char1);
        syncWord = value;
        Serial.print("0x");
        Serial.println(syncWord, HEX);
      }
    } else {
      Serial.println(" (invalid)");
      return false;
    }
  }

  inputString.remove(0, command_stop_index);
  Serial.print("  Remaining input string length: ");
  Serial.println(inputString.length());

  if (1 < inputString.length()) {
    Input_Message_Handler(inputString);
  }

  return true;
}

bool isValidValue(String stringData, bool* isHEX, bool* isFloat) {
  bool output = true;
  
  *isHEX = false;
  uint8_t counter = 0;
  if (stringData.substring(0, 2).equals("0x")) {
    *isHEX = true;
    stringData.remove(0, 2);
    stringData.toLowerCase();
  }

  uint8_t stringLength = stringData.length();
  *isFloat = false;

  for (uint8_t index = 0; index < stringLength; index += 1) {
    char charInCheck = stringData.charAt(index);

    if ('.' == charInCheck) {   // Check floating point indicator '.'
      if ((0 == index) || *isFloat || *isHEX) return false;   // String starts with '.', already has '.' present, or is a HEX number
      else *isFloat = true;
    } else if (('0' <= charInCheck) && ('9' >= charInCheck)) {
      if (*isHEX) {
        counter += 1;
        if (counter > 2) return false;
      }
    } else if (*isHEX && ('a' <= charInCheck) && ('f' >= charInCheck)) {
      counter += 1;
      if (counter > 2) return false;
    } else {  // Input string is not a (valid) number
      return false;
    }
  }

  if (*isHEX && (2 != counter)) return false;
  return output;
}

int HEX_to_int(uint8_t HEX_value) {
  if (('0' <= HEX_value) && (HEX_value <= '9')) {
    HEX_value -= '0';
  } else if (('a' <= HEX_value) && (HEX_value <= 'f')) {
    HEX_value -= 'W';
  }

  return HEX_value;
}
