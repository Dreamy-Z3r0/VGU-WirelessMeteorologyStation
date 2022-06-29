#include "LoRa_Handler.h"

void LoRaSettings(LoRa_Settings *user_settings) {
  if (user_settings->new_sf) {
    user_settings->new_sf = false;
    LoRa.setSpreadingFactor(user_settings->spreadingFactor);
  }

  if (user_settings->new_sb) {
    user_settings->new_sb = false;
    LoRa.setSignalBandwidth(user_settings->signalBandwidth);
  }

  if (user_settings->new_cr) {
    user_settings->new_cr = false;
    LoRa.setCodingRate4(user_settings->codingRate4);
  }

  if (user_settings->new_sw) {
    user_settings->new_sw = false;
    LoRa.setSyncWord(user_settings->syncWord);
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
}

void onTxDone() {
  Serial.println("TxDone");
  LoRa_rxMode();
}
