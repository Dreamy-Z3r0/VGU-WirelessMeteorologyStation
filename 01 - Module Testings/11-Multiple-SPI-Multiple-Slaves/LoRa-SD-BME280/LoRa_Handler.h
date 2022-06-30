#include <SPI.h>
#include <LoRa.h>

typedef struct {
  int spreadingFactor;
  long signalBandwidth;
  int codingRate4;
  int syncWord;

  bool new_sf;
  bool new_sb;
  bool new_cr;
  bool new_sw;
} LoRa_Settings;

typedef struct {
  String LoRa_message;
  bool gateway_messaged;
} LoRa_rx_handler;

extern LoRa_Settings LoRa_settings;
extern LoRa_rx_handler Received_over_LoRa;

void LoRaSettings();
void LoRa_rxMode();
void LoRa_txMode();
void LoRa_sendMessage(String message);
void onReceive(int packetSize);
void onTxDone();
