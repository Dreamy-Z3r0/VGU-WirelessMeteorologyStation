/***
 *** LoRa connectivity test - Simple node
 ***
 *** Microcontroller: STM32F103CBT6 @ 72MHz with remapped SPI
 ***    + U(S)ART support: "Enabled (generic 'Serial')"
 ***    + USB support (if available): "None"    
 ***    + USB speed (if available): "Low/Full Speed"
 ***    + Optimize: "Smallest (-Os default)"
 ***    + Debug symbols: "None"
 ***    + C Runtime Library: "Newlib Nano (default)"    
 ***    
 *** LoRa module: SX1278
 *** LoRa settings are input via Serial and updated with LoRaSettings(..)
 *** Initial LoRa settings:
 ***    + LoRa frequency: 433 MHz
 ***    + Spreading factor: 12    
 ***    + Signal bandwidth: 500 kHz    
 ***    + Coding rate: 4/5 
 ***    + Sync word: 0x12     
 *** 
 ***      Hardware connections
 ***  STM32  | SX1278 |  Function
 *** --------|--------|----------------
 ***   PB15  |  MOSI  | MOSI
 ***   PB14  |  MISO  | MISO
 ***   PB13  |  SCK   | CLK
 ***   PB12  |  NSS   | CS0
 *** --------|--------|----------------
 ***   PA8   |  RST   | LoRa Reset
 ***   PA11  |  DIO0  | EXTI from LoRa
 */


#include <SPI.h>
#include <LoRa.h>

#define LED_INDICATOR PC13

#define SPI2_MOSI_Pin PB15   // SPI2 MOSI pin
#define SPI2_MISO_Pin PB14   // SPI2 MISO pin
#define SPI2_SCLK_Pin PB13   // SPI2 SCLK pin
//#define SPI2_SSEL_Pin PB12   // SPI2 SSEL pin -> not used

//SPIClass SPI_2(SPI2_MOSI_Pin, SPI2_MISO_Pin, SPI2_SCLK_Pin);

#define command_start_char '?'
#define command_stop_char  '_'
#define command_separator  ':'

const long frequency = 433E6;   // LoRa frequency

const int csPin = PA12;    // SPI NCSS for LoRa
const int resetPin = PA8;  // LoRa reset
const int irqPin = PA11;   // Interrupt by LoRa

int spreadingFactor = 12;
long signalBandwidth = 500E3;
int codingRate4 = 8;
int syncWord = 0x12;

bool new_sf = false,
     new_sb = false,
     new_cr = false,
     new_sw = false;

String gateway_msg, reply, LoRaNewSettings;
bool gatewayMessaged = false,
     LoRa_newSettingsAvailable = false;
unsigned long timestamp, timestamp_lastmsg;

int rx_packetRssi;
float rx_packetSnr;
long rx_packetFrequencyError;

void setup() {
  pinMode(LED_INDICATOR, OUTPUT);
  digitalWrite(LED_INDICATOR, LOW);
  
  // Initialize Serial
  Serial.begin(9600);
  while (!Serial);

  // Initialize LoRa
  SPIClass* NewSPI = new SPIClass(SPI2_MOSI_Pin, SPI2_MISO_Pin, SPI2_SCLK_Pin);
  LoRa.setSPI(*NewSPI);
  LoRa.setPins(csPin, resetPin, irqPin);
  if (!LoRa.begin(frequency)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (!LoRa.begin(frequency)) {
      Serial.println("LoRa init failed. Check your connections.");
      delay(4000);                   
    }
  }

  // Set up LoRa parameters
  LoRaSettings();

  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);
  LoRa_rxMode();
  
  timestamp = millis();
}

void loop() {
  Serial_InputHandler();

  if (gatewayMessaged) {
    gatewayMessaged = false;

    if (gateway_msg.substring(0, gateway_msg.indexOf('=')).equals("New LoRa settings")) {
      LoRa_newSettingsAvailable = true;
      Input_Message_Handler(gateway_msg);
    }
    
    Serial.printf("\nLast message: %.3f seconds ago.\n", (millis()-timestamp_lastmsg)/1000.0, 3);
    timestamp_lastmsg = millis();
    timestamp = millis();

    Serial.print("Node received: \"");
    Serial.print(gateway_msg);
    Serial.println("\"");

    Serial.printf(" packet RSSI = %d dBm\n", rx_packetRssi);
    Serial.printf(" packet SNR = %d dB\n", rx_packetSnr);
    Serial.printf(" Frequency error = %d Hz\n", rx_packetFrequencyError);

    if (LoRa_newSettingsAvailable) {
      reply = "New LoRa settings = OK";
    } else {
      reply = "Received by node: ";
      reply += gateway_msg;
    }
    LoRa_sendMessage(reply);
  } else if (millis() - timestamp >= 15000) {
    Serial.printf("\nIdle...\n");
    Serial.printf("\nLast message: %.3f seconds ago.\n", (millis()-timestamp_lastmsg)/1000.0, 3);
    timestamp = millis();
  }

  if (new_sf | new_sb | new_cr | new_sw) {
    LoRaSettings();
  }
}

void LoRaSettings(void) {
  if (new_sf) {
    new_sf = false;
    LoRa.setSpreadingFactor(spreadingFactor);
  }

  if (new_sb) {
    new_sb = false;
    LoRa.setSignalBandwidth(signalBandwidth);
  }

  if (new_cr) {
    new_cr = false;
    LoRa.setCodingRate4(codingRate4);
  }

  if (new_sw) {
    new_sw = false;
    LoRa.setSyncWord(syncWord);
  }
}

void LoRa_rxMode(){
  LoRa.enableInvertIQ();                // active invert I and Q signals
  LoRa.receive();                       // set receive mode
}

void LoRa_txMode(){
  LoRa.idle();                          // set standby mode
  LoRa.disableInvertIQ();               // normal mode
}

void LoRa_sendMessage(String message) {
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  LoRa.print(message);                  // add payload
  LoRa.endPacket(true);                 // finish packet and send it
}

void onReceive(int packetSize) {
  gateway_msg = "";

  while (LoRa.available()) {
    gateway_msg += (char)LoRa.read();
  }

  rx_packetRssi = LoRa.packetRssi();
  rx_packetSnr = LoRa.packetSnr();
  rx_packetFrequencyError = LoRa.packetFrequencyError();

  gatewayMessaged = true;
}

void onTxDone() {
  Serial.print("TxDone: \"");
  Serial.print(reply);
  Serial.print("\"\n");
  LoRa_rxMode();
}

void Serial_InputHandler(void) {
  if (Serial.available()) {   // Check for user input
    String input = Serial.readString();
    bool validity_check = Input_Message_Handler(input);
    
    if (validity_check) {
      Serial.println("At least 1 input is available and valid.");
    } else if (input.equals("settings?")) {      // Request for current LoRa settings
      Serial.println("\nCurrent LoRa settings:");

      // LoRa frequency
      Serial.print("   + LoRa frequency: ");
      Serial.print((int)(frequency/1E6));
      Serial.println("MHz");

      // Spreading factor
      Serial.print("   + Spreading factor: ");
      Serial.println(spreadingFactor);

      // Signal bandwidth
      Serial.print("   + Signal bandwidth: ");
      if (31.25E3 == signalBandwidth)    
        Serial.print("31.25");
      else if (125E3 <= signalBandwidth) 
        Serial.print((int)(signalBandwidth/1E3));
      else 
        Serial.print((float)(signalBandwidth/1E3), 1);
      Serial.println("kHz");

      // Coding rate
      Serial.print("   + Coding rate: 4/");
      Serial.println(codingRate4);

      // Sync word
      Serial.print("   + Sync word: 0x");
      Serial.print(syncWord, HEX);
      Serial.println("\n");
    } else {
    Serial.println("Invalid input.\n");
    }
  }
}

bool Input_Message_Handler(String inputString) {
  Serial.println("\nString processing scope:");
  
  Serial.print("  String to process: ");
  Serial.print(inputString);
  
  int command_start_index = inputString.indexOf(command_start_char),
      command_stop_index = -1,
      command_separator_index = -1;

  if (0 != command_start_index) {
    if (-1 == command_start_index) {
      Serial.println("  Invalid command format: no start character.");
      return false;
    } else {
      inputString.remove(0, command_start_index);
      command_start_index = 0;
      command_separator_index = inputString.indexOf(command_separator);
      
      Serial.print("  Command starts at ");
      Serial.println(command_start_index);
    }
  } else {
    Serial.print("  Command starts at ");
    Serial.println(command_start_index);
    command_separator_index = inputString.indexOf(command_separator);
  }

  if (-1 == command_separator_index) {
    Serial.println("  Invalid command format: no colon");
    return false;
  } else {
    Serial.print("  Command separator at ");
    Serial.println(command_separator_index);
    command_stop_index = inputString.indexOf(command_stop_char);
  }    
   
  if (-1 == command_stop_index) {
    Serial.println("  Invalid command format: no stop character.");
    return false;
  } else {
    Serial.print("  Command stops at ");
    Serial.println(command_stop_index);
  }
  
  String command = inputString.substring(0, command_separator_index);

  Serial.print("  Input type: ");
  if (command.equals("?sf")) {
    Serial.print("spreading factor\n  Value: ");
    command = inputString.substring(command_separator_index+1, command_stop_index);
    bool isHEX, isFloat;
    if (isValidValue(command, &isHEX, &isFloat)) {
      if (isHEX || isFloat) {
        Serial.println(" (invalid - float or hex input)");
        return false;
      } else {
        int value = command.toInt();
        if ((6 <= value) && (12 >= value)) {
          Serial.print(spreadingFactor);
          if (spreadingFactor == value) {
            if (new_sf) Serial.println(" (duplicated input)");
            else Serial.println(" (unchanged)");
          } else {
            new_sf = true;
            spreadingFactor = value;
            Serial.print(" -> ");
            Serial.println(spreadingFactor);
            
            if (!LoRa_newSettingsAvailable) {
              LoRaNewSettings += inputString.substring(0, command_separator_index+1);
            }
          }
        } else {
          Serial.println(" (invalid - out of range)");
          return false;
        }
      }
    } else {
      Serial.println(" (invalid value)");
      return false;
    }
  } else if (command.equals("?sb")) {
    Serial.print("signal bandwidth\n  Value: ");
    command = inputString.substring(command_separator_index+1, command_stop_index);
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
            if (31.25E3 == signalBandwidth)    
              Serial.print("31.25");
            else if (125E3 <= signalBandwidth) 
              Serial.print((int)(signalBandwidth/1E3));
            else 
              Serial.print((float)(signalBandwidth/1E3), 1);
            Serial.print("kHz");

            if (signalBandwidth == value) {
              if (new_sb) Serial.println(" (duplicated input)");
              else Serial.println(" (unchanged)");
            } else {
              new_sb = true;
              signalBandwidth = value;
              Serial.print(" -> ");
              if (31.25E3 == signalBandwidth)
                Serial.print("31.25");
              else if (125E3 <= signalBandwidth)
                Serial.print((int)(signalBandwidth/1E3));
              else
                Serial.print((float)(signalBandwidth/1E3), 1);
              Serial.println("kHz");

              if (!LoRa_newSettingsAvailable) {
                LoRaNewSettings += inputString.substring(0, command_separator_index+1);
              }
            }
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
    command = inputString.substring(command_separator_index+1, command_stop_index);
    bool isHEX, isFloat;
    if (isValidValue(command, &isHEX, &isFloat)) {
      if (isHEX || isFloat) {
        Serial.println(" (invalid - float or hex input)");
        return false;
      } else {
        int value = command.toInt();
        if ((5 <= value) && (8 >= value)) {
          Serial.print("4/");
          Serial.print(codingRate4);
          if (codingRate4 == value) {
            if (new_cr) Serial.println(" (duplicated input)");
            else Serial.println(" (unchanged)");
          } else {
            codingRate4 = value;
            Serial.print(" -> 4/");
            Serial.println(codingRate4);

            if (!LoRa_newSettingsAvailable) {
              LoRaNewSettings += inputString.substring(0, command_separator_index+1);
            }
          }
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
    command = inputString.substring(command_separator_index+1, command_stop_index);
    bool isHEX, isFloat;
    if (isValidValue(command, &isHEX, &isFloat)) {
      if (!isHEX) {
        Serial.println(" (invalid - not HEX)");
      } else {
        command.toLowerCase();
        uint8_t char0 = command.charAt(2),
                char1 = command.charAt(3);
        
        int value = (HEX_to_int(char0) << 4) | HEX_to_int(char1);
        Serial.print("0x");
        Serial.print(syncWord, HEX);

        if (syncWord == value) {
          if (new_sw) Serial.println(" (duplicated input)");
          else Serial.println(" (unchanged)");
        } else {
          new_sw = true;
          syncWord = value;
          Serial.print(" -> 0x");
          Serial.println(syncWord, HEX);

          if (!LoRa_newSettingsAvailable) {
            LoRaNewSettings += inputString.substring(0, command_separator_index+1);
          }
        }
      }
    } else {
      Serial.println(" (invalid)");
      return false;
    }
  }

  if (inputString.charAt(command_stop_index+1) != '\n') {
    inputString.remove(0, command_stop_index);
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
