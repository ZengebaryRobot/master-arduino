#include <AltSoftSerial.h>

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
  AltSoftSerial *serial;
  Status status;
  unsigned long requestTime;
  const unsigned long TIMEOUT_MS = 3000;
  char buffer[256];
  int bufferIndex;
  int values[20]; // i assume max val is 20
  int valueCount;

public:
  Client(AltSoftSerial *serialPort)
  {
    serial = serialPort;
    status = IDLE;
    bufferIndex = 0;
    valueCount = 0;
  }

  void begin(long baudRate)
  {
    serial->begin(baudRate);
  }

  // clears buffer and sends the request ONLY, updates state to WAITING
  void sendRequest(const char *command)
  {
    // Clear any pending data
    while (serial->available())
    {
      serial->read();
    }

    // Send command
    serial->println(command);
    Serial.print("Sent: ");
    Serial.println(command);

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
        Serial.println("Response timeout");
        return;
      }

      while (serial->available() && bufferIndex < 255)
      {
        char c = serial->read();
        if (c == '\n')
        {
          buffer[bufferIndex] = '\0';

          Serial.print("Received: ");
          Serial.println(buffer);

          if (strncmp(buffer, "ERROR", 5) == 0)
          {
            status = ERROR;
            Serial.println("ESP32-CAM error");
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
  int *getVisionDataBlocking(Client &client, const char *command, int *valueCount, unsigned long timeout = 3000)
  {

    client.sendRequest(command);

    unsigned long startTime = millis();

    while (!client.available())
    {
      client.update();

      if (millis() - startTime > timeout)
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
    while (token && valueCount < 20)
    {
      values[valueCount++] = atoi(token);
      token = strtok(NULL, ",");
    }
  }
};