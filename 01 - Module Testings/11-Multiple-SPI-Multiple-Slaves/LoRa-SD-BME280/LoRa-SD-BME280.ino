#include "Libraries.h"
#include "Macros_and_Variables.h"


void setup() {
  Serial.begin(9600);
  while (!Serial);

  // Initialize LoRa settings
  LoRa_settings.spreadingFactor = 12;
  LoRa_settings.signalBandwidth = 500E3;
  LoRa_settings.codingRate4 = 5;
  LoRa_settings.syncWord = 0x12;

  // Update LoRa settings when the module is up and running
  LoRa_settings.new_sf = true; LoRa_settings.new_sb = true;
  LoRa_settings.new_cr = true; LoRa_settings.new_sw = true;

  // Initialize local storage for LoRa message
  Received_over_LoRa.LoRa_message = "";
  Received_over_LoRa.gateway_messaged = false;

  // Initialize LoRa
  LoRa.setSPI(SPI_2);
  LoRa.setPins(LoRa_CS_Pin, LoRa_resetPin, LoRa_irqPin);
  while (!LoRa.begin(LoRa_frequency)) {
    Serial.println("LoRa init failed. Check your connections.");
    delay(4000);                   
  }

  // Initiate BME280
  unsigned status;
  status = bme.begin();
  if (!status) {
    while (1);
  }
  delay(20);

  // Initialize BME280 settings
  BME280_userSettings.Power_Mode = Adafruit_BME280::MODE_NORMAL;
  BME280_userSettings.Temperature_Oversampling = Adafruit_BME280::SAMPLING_X2;
  BME280_userSettings.Pressure_Oversampling = Adafruit_BME280::SAMPLING_X16;
  BME280_userSettings.Humidity_Oversampling = Adafruit_BME280::SAMPLING_X1;
  BME280_userSettings.Filter_Coefficient = Adafruit_BME280::FILTER_X16;
  
  update_BME280_settings(&bme);

  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);
  LoRa_rxMode();
}

void loop() {
  Serial_InputHandler();
  
  if (LoRa_settings.new_sf | LoRa_settings.new_sb | LoRa_settings.new_cr | LoRa_settings.new_sw) {    // Update LoRa settings if new input is available and valid
    LoRaSettings();
  }
  
  if (BME280_readFlag) {
    read_BME280(&bme, &BME280_readFlag);

    Serial.print("Ambient temperature = ");
    Serial.print(BME280_dataStorage.temperature);
    Serial.println(" ºC");

    Serial.print("Relative humidity = ");
    Serial.print(BME280_dataStorage.humidity);
    Serial.println(" %RH");

    Serial.print("Atmospheric pressure = ");
    Serial.print(BME280_dataStorage.pressure);
    Serial.println(" hPa");

    Serial.println();
  }
}

void Serial_InputHandler() {
  if (Serial.available()) {   // Check for user input
    String input = Serial.readStringUntil('\n');
    long input_StringLength = input.length();

    String input_processed = input.substring(0,4);  // Identifier of input data
    if (input_processed.equals("?sf:")) {            // Input is spreading factor
      if ((5 == input_StringLength) | (6 == input_StringLength)) {
        input_StringLength = input.substring(4).toInt();
        if ((7 <= input_StringLength) && (13 >= input_StringLength)) {
          Serial.print("Spreading factor: ");
          Serial.print(LoRa_settings.spreadingFactor);
          if (LoRa_settings.spreadingFactor != (int)input_StringLength) {
            LoRa_settings.new_sf = true;
            Serial.print(" -> ");
            LoRa_settings.spreadingFactor = (int)input_StringLength;
            Serial.print(LoRa_settings.spreadingFactor);
            Serial.println("\n");
          } else {
            LoRa_settings.new_sf = false;
            Serial.println(" (unchanged)\n");
          }
        } else {
          Serial.println("Invalid spreading factor input. Allowed: 6 / 7 / 8 / 9 / 10 / 11 / 12\n");
        }
      } else {
        Serial.println("Invalid input format and/or value.\n"); 
      }
    } else if (input_processed.equals("?sb:")) {     // Input is signal bandwidth
      if ((7 <= input_StringLength) | (9 >= input_StringLength)) {
        input_StringLength = (long)(input.substring(4).toFloat() * 1000);
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
            if (31.25E3 == LoRa_settings.signalBandwidth)    
              Serial.print("31.25");
            else if (125E3 <= LoRa_settings.signalBandwidth) 
              Serial.print((int)(LoRa_settings.signalBandwidth/1E3));
            else 
              Serial.print((float)(LoRa_settings.signalBandwidth/1E3), 1);
            Serial.print("kHz");

            if (LoRa_settings.signalBandwidth == input_StringLength) {
              LoRa_settings.new_sb = false;
              Serial.println(" (unchanged)\n");
            } else {
              LoRa_settings.new_sb = true;
              LoRa_settings.signalBandwidth = input_StringLength;
              Serial.print(" -> ");
              if (31.25E3 == LoRa_settings.signalBandwidth)    
                Serial.print("31.25");
              else if (125E3 <= LoRa_settings.signalBandwidth) 
                Serial.print((int)(LoRa_settings.signalBandwidth/1E3));
              else 
                Serial.print((float)(LoRa_settings.signalBandwidth/1E3), 1);
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
    } else if (input_processed.equals("?cr:")) {     // Input is coding rate
      if (5 == input_StringLength) {
        input_StringLength = input.substring(4).toInt();
        if ((6 <= input_StringLength) && (9 >= input_StringLength)) {
          Serial.print("Coding rate: 4/");
          Serial.print(LoRa_settings.codingRate4);
          if (LoRa_settings.codingRate4 != (int)input_StringLength) {
            LoRa_settings.new_cr = true;
            Serial.print(" -> 4/");
            LoRa_settings.codingRate4 = (int)input_StringLength;
            Serial.print(LoRa_settings.codingRate4);
            Serial.println("\n");
          } else {
            LoRa_settings.new_cr = false;
            Serial.println(" (unchanged)\n");
          }
        } else {
          Serial.println("Invalid coding rate input. Allowed: 5 / 6 / 7 / 8 <-> 4/5 / 4/6 / 4/7 / 4/8\n");
        }
      } else {
        Serial.println("Invalid input format and/or value.\n"); 
      }
    } else if (input_processed.equals("?sw:")) {     // Input is sync word
      if (6 != input_StringLength) {
        Serial.println("Invalid input format and/or value.\n"); 
      } else {
        uint8_t char1 = input.charAt(4),
                char2 = input.charAt(5);
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
          Serial.print(LoRa_settings.syncWord, HEX);
          if (LoRa_settings.syncWord != ((char1 << 4) | char2)) {
            LoRa_settings.new_sw = true;
            LoRa_settings.syncWord = (char1 << 4) | char2;
            Serial.print(" -> 0x");
            Serial.print(LoRa_settings.syncWord, HEX);
            Serial.println("\n");
          } else {
            LoRa_settings.new_sw = false;
            Serial.println(" (unchanged)\n");
          }
        } else {
          Serial.println("Invalid input format and/or value.\n"); 
        }
      }
    } else if (input.equals("?settings?")) {       // Request for current LoRa settings
      Serial.println("Current LoRa settings:");

      // LoRa frequency
      Serial.print("   + LoRa frequency: ");
      Serial.print((int)(LoRa_frequency/1E6));
      Serial.println("E6");

      // Spreading factor
      Serial.print("   + Spreading factor: ");
      Serial.println(LoRa_settings.spreadingFactor);

      // Signal bandwidth
      Serial.print("   + Signal bandwidth: ");
      if (31.25E3 == LoRa_settings.signalBandwidth)    
        Serial.print("31.25");
      else if (125E3 <= LoRa_settings.signalBandwidth) 
        Serial.print((int)(LoRa_settings.signalBandwidth/1E3));
      else 
        Serial.print((float)(LoRa_settings.signalBandwidth/1E3), 1);
      Serial.println("E3");

      // Coding rate
      Serial.print("   + Coding rate: 4/");
      Serial.println(LoRa_settings.codingRate4);

      // Sync word
      Serial.print("   + Sync word: 0x");
      Serial.print(LoRa_settings.syncWord, HEX);
      Serial.println("\n");
    } else {    // Invalid input
      Serial.println("Invalid input.\n");
    }
  }
}

//void 
