#include "LoRa_Handler.h"

void LoRaSettings() {
  if (LoRa_settings.new_sf) {
    LoRa_settings.new_sf = false;
    LoRa.setSpreadingFactor(LoRa_settings.spreadingFactor);
  }

  if (LoRa_settings.new_sb) {
    LoRa_settings.new_sb = false;
    LoRa.setSignalBandwidth(LoRa_settings.signalBandwidth);
  }

  if (LoRa_settings.new_cr) {
    LoRa_settings.new_cr = false;
    LoRa.setCodingRate4(LoRa_settings.codingRate4);
  }

  if (LoRa_settings.new_sw) {
    LoRa_settings.new_sw = false;
    LoRa.setSyncWord(LoRa_settings.syncWord);
  }
}

void LoRa_new_sf_handler(int new_sf_value) {
  if ((6 <= new_sf_value) && (12 >= new_sf_value)) {
    Serial.print("LoRa spreading factor: ");
    Serial.print(LoRa_settings.spreadingFactor);
    if (LoRa_settings.spreadingFactor == new_sf_value) {
      LoRa_settings.new_sf = false;
      Serial.println(" (unchanged)\n");
    } else {
      LoRa_settings.new_sf = true;
      LoRa_settings.spreadingFactor = new_sf_value;
      Serial.print(" -> ");
      Serial.print(LoRa_settings.spreadingFactor);
      Serial.println("\n");
    }
  } else {
    LoRa_settings.new_sf = false;
    Serial.print("Invalid LoRa spreading factor input: ");
    Serial.print(new_sf_value);
    Serial.println("\n");
  }
}

void LoRa_new_sf_handler(long new_sb_value) {
  switch (new_sb_value) {
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
      Serial.print("LoRa signal bandwidth: ");
      if (31.25E3 == LoRa_settings.signalBandwidth)    
        Serial.print("31.25");
      else if (125E3 <= LoRa_settings.signalBandwidth) 
        Serial.print((int)(LoRa_settings.signalBandwidth/1E3));
      else 
        Serial.print((float)(LoRa_settings.signalBandwidth/1E3), 1);
      Serial.print("kHz");

      if (LoRa_settings.signalBandwidth == new_sb_value) {
        LoRa_settings.new_sb = false;
        Serial.println(" (unchanged)\n");
      } else {
        LoRa_settings.new_sb = true;
        LoRa_settings.signalBandwidth = new_sb_value;
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
      LoRa_settings.new_sb = false;
      Serial.print("Invalid LoRa signal bandwidth input: ");
      Serial.print(((float)new_sb_value/1E3), 2);
      Serial.println("kHz\n");
      break;
    }
  }
}

void LoRa_new_cr_handler(int new_cr_value) {
  if ((5 <= new_cr_value) && (8 >= new_cr_value)) {
    Serial.print("LoRa coding rate: 4/");
    Serial.print(LoRa_settings.codingRate4);
    if (LoRa_settings.codingRate4 == new_cr_value) {
      LoRa_settings.new_cr = false;
      Serial.println(" (unchanged)\n");
    } else {
      LoRa_settings.new_cr = true;
      LoRa_settings.codingRate4 = new_cr_value;
      Serial.print(" -> 4/");
      Serial.print(LoRa_settings.codingRate4);
      Serial.println("\n");
    }
  } else {
    LoRa_settings.new_cr = false;
    Serial.print("Invalid LoRa coding rate input: 4/");
    Serial.print(new_cr_value);
    Serial.println("\n");
  }
}

void LoRa_new_sw_handler(int new_sw_value) {
  if ((0x00 <= new_sw_value) && (0xFF >= new_sw_value)) {
    Serial.print("LoRa sync word: 0x");
    Serial.print(LoRa_settings.syncWord, HEX);
    if (LoRa_settings.syncWord == new_sw_value) {
      LoRa_settings.new_sw = false;
      Serial.println(" (unchanged)\n");
    } else {
      LoRa_settings.new_sw = true;
      LoRa_settings.syncWord = new_sw_value;
      Serial.print(" -> 0x");
      Serial.print(LoRa_settings.syncWord, HEX);
      Serial.println("\n");
    }
  } else {
    LoRa_settings.new_sw = false;
    Serial.print("Invalid LoRa sync word input: 0x");
    Serial.print(new_sw_value, HEX);
    Serial.println("\n");
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
  Received_over_LoRa.LoRa_message = "";

  while (LoRa.available()) {
    Received_over_LoRa.LoRa_message += (char)LoRa.read();
  }

  Serial.print("Node Receive: ");
  Serial.println(Received_over_LoRa.LoRa_message);

  Received_over_LoRa.gateway_messaged = true;
}

void onTxDone() {
  Serial.println("TxDone");
  LoRa_rxMode();
}

LoRa_Settings LoRa_settings;
LoRa_rx_handler Received_over_LoRa;
