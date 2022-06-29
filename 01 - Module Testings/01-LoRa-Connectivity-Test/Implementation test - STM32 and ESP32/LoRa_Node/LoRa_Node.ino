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

#define SPI2_MOSI_Pin PB15   // SPI2 MOSI pin
#define SPI2_MISO_Pin PB14   // SPI2 MISO pin
#define SPI2_SCLK_Pin PB13   // SPI2 SCLK pin
//#define SPI2_SSEL_Pin PB12   // SPI2 SSEL pin -> not used

SPIClass SPI_2(SPI2_MOSI_Pin, SPI2_MISO_Pin, SPI2_SCLK_Pin);

const long frequency = 433E6;   // LoRa frequency

const int csPin = PB12;    // SPI NCSS for LoRa
const int resetPin = PA8;  // LoRa reset
const int irqPin = PA11;   // Interrupt by LoRa

int spreadingFactor = 12;
long signalBandwidth = 500E3;
int codingRate4 = 5;
int syncWord = 0x12;

bool new_sf = true,
     new_sb = true,
     new_cr = true,
     new_sw = true;

bool gatewayMessaged = false;
unsigned long timestamp;

void setup() {
  // Initialize Serial
  Serial.begin(9600);
  while (!Serial);

  // Initialize LoRa
  LoRa.setSPI(SPI_2);
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
  static int counter = 10;

  Serial_InputHandler();
  if (new_sf | new_sb | new_cr | new_sw) {
    LoRaSettings();
  }

  if (gatewayMessaged) {
    gatewayMessaged = false;
    String message = "Node counter = ";
    message += String(counter);

    LoRa_sendMessage(message);
    
    counter -= 1;
    if (0 > counter) counter = 10;
  }
  else if (millis() - timestamp >= 5000) {
    Serial.println("Idle...");
    timestamp = millis();
  }
}

void Serial_InputHandler() {
  if (Serial.available()) {   // Check for user input
    String input = Serial.readStringUntil('\n');
    long input_StringLength = input.length();

    String input_processed = input.substring(0,3);  // Identifier of input data
    if (input_processed.equals("sf:")) {            // Input is spreading factor
      if ((4 == input_StringLength) | (5 == input_StringLength)) {
        input_StringLength = input.substring(3).toInt();
        if ((6 <= input_StringLength) && (12 >= input_StringLength)) {
          Serial.print("Spreading factor: ");
          Serial.print(spreadingFactor);
          if (spreadingFactor != (int)input_StringLength) {
            new_sf = true;
            Serial.print(" -> ");
            spreadingFactor = (int)input_StringLength;
            Serial.print(spreadingFactor);
            Serial.println("\n");
          } else {
            new_sf = false;
            Serial.println(" (unchanged)\n");
          }
        } else {
          Serial.println("Invalid spreading factor input. Allowed: 6 / 7 / 8 / 9 / 10 / 11 / 12\n");
        }
      } else {
        Serial.println("Invalid input format and/or value.\n"); 
      }
    } else if (input_processed.equals("sb:")) {     // Input is signal bandwidth
      if ((6 <= input_StringLength) | (8 >= input_StringLength)) {
        input_StringLength = (long)(input.substring(3).toFloat() * 1000);
        switch (input_StringLength) {
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
            Serial.print("Signal bandwidth: ");
            if (31.25E3 == signalBandwidth)    
              Serial.print("31.25");
            else if (125E3 <= signalBandwidth) 
              Serial.print((int)(signalBandwidth/1E3));
            else 
              Serial.print((float)(signalBandwidth/1E3), 1);
            Serial.print("kHz");

            if (signalBandwidth == input_StringLength) {
              new_sb = false;
              Serial.println(" (unchanged)\n");
            } else {
              new_sb = true;
              signalBandwidth = input_StringLength;
              Serial.print(" -> ");
              if (31.25E3 == signalBandwidth)    
                Serial.print("31.25");
              else if (125E3 <= signalBandwidth) 
                Serial.print((int)(signalBandwidth/1E3));
              else 
                Serial.print((float)(signalBandwidth/1E3), 1);
              Serial.println("kHz");
            }
            break;
          }
          default: {
            Serial.print("Invalid signal bandwidth input. ");
            Serial.println("Allowed: 7.8 / 10.4 / 15.6 / 20.8 / 31.25 / 41.7 / 62.5 / 125 / 250 / 500 (E3)\n");
          }
        }
      } else {
        Serial.println("Invalid input format and/or value.\n"); 
      }
    } else if (input_processed.equals("cr:")) {     // Input is coding rate
      if (4 == input_StringLength) {
        input_StringLength = input.substring(3).toInt();
        if ((5 <= input_StringLength) && (8 >= input_StringLength)) {
          Serial.print("Coding rate: 4/");
          Serial.print(codingRate4);
          if (codingRate4 != (int)input_StringLength) {
            new_cr = true;
            Serial.print(" -> 4/");
            codingRate4 = (int)input_StringLength;
            Serial.print(codingRate4);
            Serial.println("\n");
          } else {
            new_cr = false;
            Serial.println(" (unchanged)\n");
          }
        } else {
          Serial.println("Invalid coding rate input. Allowed: 5 / 6 / 7 / 8 <-> 4/5 / 4/6 / 4/7 / 4/8\n");
        }
      } else {
        Serial.println("Invalid input format and/or value.\n"); 
      }
    } else if (input_processed.equals("sw:")) {     // Input is sync word
      if (5 != input_StringLength) {
        Serial.println("Invalid input format and/or value.\n"); 
      } else {
        uint8_t char1 = input.charAt(3),
                char2 = input.charAt(4);
        bool char1_ok = true,
             char2_ok = true;

        if (('0' <= char1) && (char1 <= '9')) {
          char1 -= '0';
        } else if (('a' <= char1) && (char1 <= 'f')) {
          char1 -= 'W';
        } else if (('A' <= char1) && (char1 <= 'F')) {
          char1 -= '7';
        } else {
          char1_ok = false;
        }

        if (('0' <= char2) && (char2 <= '9')) {
          char2 -= '0';
        } else if (('a' <= char2) && (char2 <= 'f')) {
          char2 -= 'W';
        } else if (('A' <= char2) && (char2 <= 'F')) {
          char2 -= '7';
        } else {
          char2_ok = false;
        }

        if (char1_ok && char2_ok) {
          Serial.print("Sync word: 0x");
          Serial.print(syncWord, HEX);
          if (syncWord != ((char1 << 4) | char2)) {
            new_sw = true;
            syncWord = (char1 << 4) | char2;
            Serial.print(" -> 0x");
            Serial.print(syncWord, HEX);
            Serial.println("\n");
          } else {
            new_sw = false;
            Serial.println(" (unchanged)\n");
          }
        } else {
          Serial.println("Invalid input format and/or value.\n"); 
        }
      }
    } else if (input.equals("settings?")) {       // Request for current LoRa settings
      Serial.println("Current LoRa settings:");

      // LoRa frequency
      Serial.print("   + LoRa frequency: ");
      Serial.print((int)(frequency/1E6));
      Serial.println("E6");

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
      Serial.println("E3");

      // Coding rate
      Serial.print("   + Coding rate: 4/");
      Serial.println(codingRate4);

      // Sync word
      Serial.print("   + Sync word: 0x");
      Serial.print(syncWord, HEX);
      Serial.println("\n");
    } else {    // Invalid input
      Serial.println("Invalid input.\n");
    }
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
  String message = "";

  while (LoRa.available()) {
    message += (char)LoRa.read();
  }

  Serial.print("Node Receive: ");
  Serial.println(message);

  gatewayMessaged = true;
}

void onTxDone() {
  Serial.println("TxDone");
  LoRa_rxMode();
}
