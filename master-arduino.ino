#include <Wire.h>
#include <AltSoftSerial.h>
#include "Client.h"
#include <LiquidCrystal_I2C.h>

#define I2C_ADDRESS_MASTER 0x10
#define I2C_ADDRESS_LCD 0x27
#define TEXT_ARG_SIZE 64

enum
{
  CMD_CAMERA = 1,
  CMD_DISPLAY = 2,
  CMD_MOVE_ARM = 3
};

AltSoftSerial espSerial; // 8 (RX) & 9 (TX)
Client camClient(&espSerial);

volatile bool newCommand = false;
volatile uint8_t commandType = 0;
volatile int intArgs[6];
volatile uint8_t intArgCount = 0;

volatile char textArgBuf[TEXT_ARG_SIZE];
volatile uint8_t textArgLen = 0;

// Response buffer
char responseBuf[128];
uint8_t responseLen = 0;
volatile bool readyToReply = false;

// LCD
LiquidCrystal_I2C lcd(I2C_ADDRESS_LCD, 16, 2);

void receiveEvent(int howMany)
{
  if (howMany < 1)
    return;

  commandType = Wire.read();
  intArgCount = 0;
  textArgLen = 0;

  switch (commandType)
  {
  case CMD_CAMERA:
  case CMD_DISPLAY:
    while (Wire.available() && textArgLen < TEXT_ARG_SIZE - 1)
      textArgBuf[textArgLen++] = Wire.read();
    textArgBuf[textArgLen] = '\0';
    break;

  case CMD_MOVE_ARM:
    while (Wire.available() && intArgCount < 6)
      intArgs[intArgCount++] = Wire.read();
    break;

  default:
    while (Wire.available())
      Wire.read();
    break;
  }

  newCommand = true;
}

void requestEvent()
{
  if (readyToReply)
  {
    Wire.write(responseLen);
    Wire.write((uint8_t *)responseBuf, responseLen);
    readyToReply = false;
  }
  else
  {
    Wire.write((uint8_t)0);
  }
}

void showOnDisplay(const String &txt)
{
  Serial.print("Displaying: ");
  Serial.println(txt);

  lcd.clear();
  delay(100);

  lcd.setCursor(0, 0);

  if (txt.length() > 16)
  {
    lcd.print(txt.substring(0, 16));

    lcd.setCursor(0, 1);
    lcd.print(txt.length() > 32 ? txt.substring(16, 32) : txt.substring(16));
  }
  else
  {
    lcd.print(txt);
  }
}

void moveArmTo(int base, int shoulder, int elbow, int wrist, int grip)
{
  Serial.print("Moving arm to: ");
  Serial.print(base);
  Serial.print(", ");
  Serial.print(shoulder);
  Serial.print(", ");
  Serial.print(elbow);
  Serial.print(", ");
  Serial.print(wrist);
  Serial.print(", ");
  Serial.println(grip);
}

void setup()
{
  // LCD init
  lcd.init();
  lcd.clear();
  lcd.backlight();
  showOnDisplay("    Zengebary       loading...");

  Serial.begin(9600);
  delay(500);

  camClient.begin(9600);
  delay(200);

  Wire.begin(I2C_ADDRESS_MASTER);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

  showOnDisplay("    Zengebary         ready");
  Serial.println("Master ready");
}

void loop()
{
  camClient.update();

  if (newCommand)
  {
    newCommand = false;
    responseLen = 0;

    String textArg = String((char *)textArgBuf);

    switch (commandType)
    {
    case CMD_CAMERA:
    {
      int valCount;
      int *vals = camClient.getVisionDataBlocking(camClient, textArg.c_str(), &valCount, 3000);
      if (vals && valCount > 0)
      {
        int p = 0;
        for (int i = 0; i < valCount; i++)
          p += snprintf(responseBuf + p, sizeof(responseBuf) - p, "%d%s", vals[i], (i < valCount - 1 ? "," : ""));

        responseLen = p;
      }
      else
      {
        responseLen = snprintf(responseBuf, sizeof(responseBuf), "ERROR");
      }
      break;
    }

    case CMD_DISPLAY:
      if (textArg.length() > 32)
      {
        responseLen = snprintf(responseBuf, sizeof(responseBuf), "ERROR");
      }
      else
      {
        showOnDisplay(textArg);
        responseLen = snprintf(responseBuf, sizeof(responseBuf), "OK");
      }
      break;

    case CMD_MOVE_ARM:
      if (intArgCount == 5)
      {
        moveArmTo(intArgs[0], intArgs[1], intArgs[2], intArgs[3], intArgs[4]);
        responseLen = snprintf(responseBuf, sizeof(responseBuf), "OK");
      }
      else
      {
        responseLen = snprintf(responseBuf, sizeof(responseBuf), "ERROR");
      }
      break;

    default:
      responseLen = snprintf(responseBuf, sizeof(responseBuf), "ERROR");
      break;
    }

    readyToReply = true;
  }

  delay(5);
}
