#include <HardwareSerial.h>
#include "DEFs.h"

class Client
{
public:
  enum Status
  {
    IDLE,
    WAITING,
    DONE,
    ERROR
  };

private:
  HardwareSerial *cameraSerial;

  Status status;
  unsigned long requestTime;
  const unsigned long TIMEOUT_MS = 5000;
  char buffer[256];
  int bufferIndex;
  int values[30];
  int valueCount;

public:
  Client(HardwareSerial *cameraSerial)
  {
    this->cameraSerial = cameraSerial;

    status = IDLE;
    bufferIndex = 0;
    valueCount = 0;
  }

  void begin(long baudRate)
  {
    cameraSerial->begin(baudRate);
  }

  // clears buffer and sends the request ONLY, updates state to WAITING
  void sendRequest(const char *command)
  {
    // Clear any pending data
    while (cameraSerial->available())
    {
      cameraSerial->read();
    }

    // Send command
    cameraSerial->println(command);

    requestTime = millis();
    status = WAITING;
    bufferIndex = 0;
  }

  // update every loop, this is what drives the state machine, if it is 'WAITING', checks for data or timeout, provides the raw response in buffer, and the ints in vals
  void update()
  {
    if (status == WAITING)
    {
      // Check for timeout
      if (millis() - requestTime > TIMEOUT_MS)
      {
        status = ERROR;
        return;
      }

      while (cameraSerial->available() && bufferIndex < 255)
      {
        char c = cameraSerial->read();
        if (c == '\n')
        {
          buffer[bufferIndex] = '\0';

          if (strncmp(buffer, "ERROR", 5) == 0)
          {
            status = ERROR;
          }
          else
          {
            parseCSV();
            status = DONE;
          }
          return;
        }
        buffer[bufferIndex++] = c;
      }
    }
  }

  // Blocking function that WAITS for response
  int *getVisionDataBlocking(Client &client, const char *command, int *valueCount)
  {
    client.sendRequest(command);

    unsigned long startTime = millis();

    while (!client.available())
    {
      client.update();

      if (millis() - startTime > TIMEOUT_MS)
      {
        *valueCount = 0;
        return nullptr;
      }

      delay(10);
    }

    // We have a response
    if (client.getStatus() == Client::DONE)
    {
      *valueCount = client.getValueCount();
      int *result = client.getData();
      client.reset();
      return result;
    }
    else
    {
      // Error occurred
      *valueCount = 0;
      client.reset();
      return nullptr;
    }
  }

  // check if there is data, use this to check for either ERROR or vals array
  bool available()
  {
    return (status == DONE || status == ERROR);
  }

  // Get the current status
  Status getStatus()
  {
    return status;
  }

  // Reset the state machine
  void reset()
  {
    status = IDLE;
  }

  // Get the parsed data array
  int *getData()
  {
    return values;
  }

  // Get number of values parsed
  int getValueCount()
  {
    return valueCount;
  }

  // Get raw response
  const char *getRawResponse()
  {
    return buffer;
  }

private:
  // Parse CSV data into the values array
  void parseCSV()
  {
    valueCount = 0;

    char tempBuffer[256];
    strncpy(tempBuffer, buffer, sizeof(tempBuffer));
    tempBuffer[sizeof(tempBuffer) - 1] = '\0';

    char *token = strtok(tempBuffer, ",");
    while (token && valueCount < 30)
    {
      values[valueCount++] = atoi(token);
      token = strtok(NULL, ",");
    }
  }
};