#include <Arduino.h>

#include "WifiTracking.h"
#include "Communication.h"

typedef enum
{
  DISCOVERY,
  IDLE,
  HEARTBEAT,
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
unsigned long timerHeartStart;
unsigned long timerHeartDelay = 10000;
unsigned long timerHeartTimeoutStart;
unsigned long timerHeartTimeoutDelay = 30000;

int macCount = 0;
int macIndex = 0;

bool receivedMessage = false;
bool resendMessage = false;

void MergeStrings(char *stringA, char *stringB, char **buffer)
{
  size_t sizeA = strlen(stringA);
  size_t sizeB = strlen(stringB);
  size_t size = sizeof(char) * (sizeA + sizeB + 1);

  *buffer = (char *)malloc(size);
  memcpy(*buffer, stringA, sizeA);
  memcpy(*buffer + sizeA, stringB, sizeB);
  (*buffer)[sizeA + sizeB] = '\0';
}

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
      char *node = "NodeMcu:";
      uint32_t id = ESP.getChipId();
      char idBuffer[64];
      sprintf(idBuffer, "%d", id);
      char* ptrMessage;
      MergeStrings(node, idBuffer, &ptrMessage);
      SendMessage(ptrMessage);
      Serial.println("");
      free(ptrMessage);
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
    else if (millis() - timerHeartStart > timerHeartDelay)
    {
      SendMessage("HEARTBEAT");
      Serial.println("");
      state = HEARTBEAT;
      timerHeartStart = millis();
    }
    break;

  case HEARTBEAT:
  if (Serial.available() > 0)
  {
   if (eventState == ACK)
    {
      state = IDLE;
      timerHeartTimeoutStart = millis();
    }
    else if (millis() - timerHeartTimeoutStart > timerHeartTimeoutDelay)
    {
      state = DISCOVERY;
    }
  }
  else
  {
    state = DISCOVERY;
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
      char *ptrMessage = NULL;
      MergeStrings(mac, address, &ptrMessage);
      SendMessage(ptrMessage);
      Serial.println("");
      free(ptrMessage);
      receivedMessage = false;
      resendMessage = false;
    }
    else if (macIndex >= macCount)
    {
      SendMessage("FINISH");
      macIndex = 0;
      Serial.println("");
      state = IDLE;
    }
    break;

  default:
    break;
  }
}