#include <Arduino.h>

#include "WifiTracking.h"
#include "Communication.h"

typedef enum
{
  DISCOVERY,
  IDLE,
  COLLECTING,
  SENDING
} wifiTrackerState;

typedef enum
{
  NOEVENT,
  DISCOVERD,
  COLLECT,
  SEND,
  ACK,
  NACK
} stateEvents;

char command[64];

unsigned long timerStart;
unsigned long timerDelay = 10000;

int macCount = 0;
int macIndex = 0;

bool receivedMessage = false;
bool resendMessage = false;

void setup()
{
  Serial.begin(115200);

  SetupWifiTracking();
}

void loop()
{
  static wifiTrackerState state = DISCOVERY;
  stateEvents eventState = NOEVENT;

  ReceiveMessage();

  memset(command, 0, sizeof(command));

  if (GetMessage(command, 64) == 1)
  {
    if (strcmp(command, "DISCOVERY") == 0)
    {
      eventState = DISCOVERD;
    }
    else if (strcmp(command, "COLLECT") == 0)
    {
      eventState = COLLECT;
    }
    else if (strcmp(command, "SEND") == 0)
    {
      eventState = SEND;
    }
    else if (strcmp(command, "ACK") == 0)
    {
      eventState = ACK;
    }
    else if (strcmp(command, "NACK") == 0)
    {
      eventState = NACK;
    }
    else
    {
      SendMessage("NACK");
      Serial.println("");
    }

    ClearMessage();
  }

  switch (state)
  {
  case DISCOVERY:
    if (eventState == DISCOVERD)
    {
      SendMessage("NodeMcu");
      Serial.println("");
    }
    else if (eventState == ACK)
    {
      state = IDLE;
    }
    break;

  case IDLE:
    if (eventState == COLLECT)
    {
      state = COLLECTING;
      SendMessage("ACK");
      Serial.println("");
    }
    else if (eventState == SEND)
    {
      receivedMessage = true;
      SendMessage("ACK");
      Serial.println("");
      state = SENDING;
    }
    break;

  case COLLECTING:
    if (!IsWifiTracking())
    {
      StartWifiTracking();
      timerStart = millis();
    }
    else if (millis() - timerStart > timerDelay)
    {
      EndWifiTracking();
      macCount = GetMacCount();
      char buf[32];
      sprintf(buf, "DONE:%d", macCount);
      SendMessage(buf);
      Serial.println("");
      state = IDLE;
    }
    break;

  case SENDING:
    if (eventState == ACK)
    {
      macIndex++;
      receivedMessage = true;
    }
    else if (eventState == NACK)
    {
      resendMessage = true;
    }

    if (macIndex < macCount && (receivedMessage || resendMessage))
    {
      char *mac = "MAC:";
      char *address = GetMacAddress(macIndex);
      size_t sizeA = strlen(mac);
      size_t sizeB = strlen(address);
      size_t size = sizeof(char) * (sizeA + sizeB + 1);
      char *message = (char *)malloc(size);
      memcpy(message, mac, sizeA);
      memcpy(message + sizeA, address, sizeB);
      message[sizeA + sizeB] = '\0';
      SendMessage(message);
      Serial.println("");
      free(message);
      receivedMessage = false;
      resendMessage = false;
    }
    else if (macIndex >= macCount)
    {
      SendMessage("FINISH");
      Serial.println("");
      state = IDLE;
    }
    break;

  default:
    break;
  }
}