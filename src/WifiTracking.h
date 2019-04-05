#ifndef WIFITRACKING_H
#define WIFITRACKING_H

#include <Arduino.h>

extern "C"
{
#include <user_interface.h>
}

#define DISABLE 0
#define ENABLE 1

#define DATA_LENGTH 112

#define TYPE_MANAGEMENT 0x00
#define SUBTYPE_PROBE_REQUEST 0x04

struct RxControl
{
  signed rssi : 8; // signal intensity of packet
  unsigned rate : 4;
  unsigned is_group : 1;
  unsigned : 1;
  unsigned sig_mode : 2;       // 0:is 11n packet; 1:is not 11n packet;
  unsigned legacy_length : 12; // if not 11n packet, shows length of packet.
  unsigned damatch0 : 1;
  unsigned damatch1 : 1;
  unsigned bssidmatch0 : 1;
  unsigned bssidmatch1 : 1;
  unsigned MCS : 7;        // if is 11n packet, shows the modulation and code used (range from 0 to 76)
  unsigned CWB : 1;        // if is 11n packet, shows if is HT40 packet or not
  unsigned HT_length : 16; // if is 11n packet, shows length of packet.
  unsigned Smoothing : 1;
  unsigned Not_Sounding : 1;
  unsigned : 1;
  unsigned Aggregation : 1;
  unsigned STBC : 2;
  unsigned FEC_CODING : 1; // if is 11n packet, shows if is LDPC packet or not.
  unsigned SGI : 1;
  unsigned rxend_state : 8;
  unsigned ampdu_cnt : 8;
  unsigned channel : 4; //which channel this packet in.
  unsigned : 12;
};

struct SnifferPacket
{
  struct RxControl rx_ctrl;
  uint8_t data[DATA_LENGTH];
  uint16_t cnt;
  uint16_t len;
};

// Declare each custom function (excluding built-in, such as setup and loop) before it will be called.
// https://docs.platformio.org/en/latest/faq.html#convert-arduino-file-to-c-manually
void showMetadata(SnifferPacket *snifferPacket);
void ICACHE_FLASH_ATTR sniffer_callback(uint8_t *buffer, uint16_t length);
void getMAC(char *addr, uint8_t *data, uint16_t offset);
void channelHop();
void SetupWifiTracking(void);

void StartWifiTracking(void);
bool IsWifiTracking(void);
void EndWifiTracking(void);

int GetMacCount(void);
char* GetMacAddress(int index);

#endif