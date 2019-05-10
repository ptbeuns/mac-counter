#include "WifiTracking.h"

int macAddressCount;
char macAddresses[1024][18];
bool collectData;

void showMetadata(SnifferPacket *snifferPacket)
{
    if (collectData)
    {
        unsigned int frameControl = ((unsigned int)snifferPacket->data[1] << 8) + snifferPacket->data[0];

        uint8_t frameType = (frameControl & 0b0000000000001100) >> 2;
        uint8_t frameSubType = (frameControl & 0b0000000011110000) >> 4;

        // Only look for probe request packets
        if (frameType != TYPE_MANAGEMENT ||
            frameSubType != SUBTYPE_PROBE_REQUEST)
            return;

        char addr[] = "00-00-00-00-00-00";
        getMAC(addr, snifferPacket->data, 10);

        for (int i = 0; i < 18; i++)
        {
            macAddresses[macAddressCount][i] = addr[i];
        }

        macAddressCount++;
    }
}

/**
 * Callback for promiscuous mode
 */
void ICACHE_FLASH_ATTR sniffer_callback(uint8_t *buffer, uint16_t length)
{
    struct SnifferPacket *snifferPacket = (struct SnifferPacket *)buffer;
    showMetadata(snifferPacket);
}

void getMAC(char *addr, uint8_t *data, uint16_t offset)
{
    sprintf(addr, "%02x:%02x:%02x:%02x:%02x:%02x", data[offset + 0], data[offset + 1], data[offset + 2], data[offset + 3], data[offset + 4], data[offset + 5]);
}

#define CHANNEL_HOP_INTERVAL_MS 1000
static os_timer_t channelHop_timer;

/**
 * Callback for channel hoping
 */
void channelHop()
{
    // hoping channels 1-13
    uint8 new_channel = wifi_get_channel() + 1;
    if (new_channel > 13)
    {
        new_channel = 1;
    }
    wifi_set_channel(new_channel);
}

void SetupWifiTracking(void)
{
    // set the WiFi chip to "promiscuous" mode aka monitor mode
    delay(10);
    wifi_set_opmode(STATION_MODE);
    wifi_set_channel(1);
    wifi_promiscuous_enable(DISABLE);
    delay(10);
    wifi_set_promiscuous_rx_cb(sniffer_callback);
    delay(10);
    wifi_promiscuous_enable(ENABLE);

    // setup the channel hoping callback timer
    os_timer_disarm(&channelHop_timer);
    os_timer_setfn(&channelHop_timer, (os_timer_func_t *)channelHop, NULL);
    os_timer_arm(&channelHop_timer, CHANNEL_HOP_INTERVAL_MS, 1);
}

void StartWifiTracking(void)
{
    macAddressCount = 0;
    collectData = true;
}

bool IsWifiTracking(void)
{
    return collectData;
}

void EndWifiTracking(void)
{
    collectData = false;
}

int GetMacCount(void)
{
    return macAddressCount;
}

char *GetMacAddress(int index)
{
    if (index <= macAddressCount)
    {
        return macAddresses[index];
    }
}