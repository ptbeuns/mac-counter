#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <Arduino.h>

#define MESSAGE_BUFFER_SIZE 64

typedef enum
{
    NONE,
    START,
    END
} messageState;


void ReceiveMessage(void);
int GetMessage(char* buffer, int bufferSize);
int SendMessage(char* buffer);
int ClearMessage(void);

#endif