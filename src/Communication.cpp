#include "Communication.h"

messageState mState = NONE;
char messageBuffer[MESSAGE_BUFFER_SIZE];
int messageIndex = 0;

void ReceiveMessage(void)
{
    if (Serial)
    {
        if (Serial.available())
        {
            int data = Serial.read();

            if (mState == NONE && data == '#')
            {
                mState = START;
            }
            else if (mState == START && data == '$')
            {
                messageBuffer[messageIndex] = NULL;
                mState = END;
            }

            if (mState == START && data != '#' && data != '$')
            {
                messageBuffer[messageIndex] = (char)data;
                messageIndex++;
                if (messageIndex > MESSAGE_BUFFER_SIZE)
                {
                    mState = END;
                }
            }
        }
    }
}

int GetMessage(char *buffer, int bufferSize)
{
    if (buffer == NULL || bufferSize < messageIndex)
    {
        return -1;
    }

    if (mState == END)
    {
        for (size_t i = 0; i < messageIndex; i++)
        {
            buffer[i] = messageBuffer[i];
        }

        return 1;
    }
    else
    {
        return 0;
    }
}

int ClearMessage(void)
{
    messageIndex = 0;
    memset(messageBuffer, 0, sizeof(messageBuffer));
    mState = NONE;
}

int SendMessage(char *buffer)
{
    if (buffer == NULL)
    {
        return -1;
    }

    Serial.print("#");
    Serial.print(buffer);
    Serial.print("$");

    return 0;
}