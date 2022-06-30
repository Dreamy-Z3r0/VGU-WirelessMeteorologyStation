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
