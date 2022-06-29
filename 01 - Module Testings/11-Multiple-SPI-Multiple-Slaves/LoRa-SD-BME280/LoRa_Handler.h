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

void LoRaSettings(LoRa_Settings *user_settings);
void LoRa_rxMode();
void LoRa_txMode();
void LoRa_sendMessage(String message);
void onReceive(int packetSize);
void onTxDone();
