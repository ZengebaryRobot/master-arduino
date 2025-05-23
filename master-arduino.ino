#include "DEFs.h"
#include <Wire.h>

#if ENABLE_DISPLAY
#include <LiquidCrystal_I2C.h>
#endif

#include "Client.h"
#include "Arm.h"

enum
{
  CMD_CAMERA = 1,
  CMD_DISPLAY = 2,
  CMD_MOVE_ARM = 3,
  CMD_MOVE_ARM_JOINT = 4
};

Client camClient(&Serial);
Arm arm;

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
#if ENABLE_DISPLAY
LiquidCrystal_I2C lcd(I2C_ADDRESS_LCD, 16, 2);
#endif

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

  case CMD_MOVE_ARM_JOINT:
    while (Wire.available() && intArgCount < 3)
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
#if ENABLE_DISPLAY
  lcd.clear();
  delay(200);

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
#endif
}

void setup()
{

  Wire.begin(I2C_ADDRESS_MASTER);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  // LCD init
#if ENABLE_DISPLAY
  lcd.init();
  lcd.clear();
  lcd.backlight();
#endif
  showOnDisplay("    Zengebary       loading...");

  camClient.begin(9600);
  delay(200);

  arm.initializeArm();
  showOnDisplay("    Zengebary         ready");
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
      int *vals = camClient.getVisionDataBlocking(camClient, textArg.c_str(), &valCount);
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
#if ENABLE_DISPLAY
      if (textArg.length() > 32)
      {
        responseLen = snprintf(responseBuf, sizeof(responseBuf), "ERROR");
      }
      else
      {
        showOnDisplay(textArg);
        responseLen = snprintf(responseBuf, sizeof(responseBuf), "OK");
      }
#else
      responseLen = snprintf(responseBuf, sizeof(responseBuf), "OK");
#endif
      break;

    case CMD_MOVE_ARM:
      if (intArgCount == 5)
      {
        arm.moveArmTo(intArgs[0], intArgs[1], intArgs[2], intArgs[3], intArgs[4]);
        responseLen = snprintf(responseBuf, sizeof(responseBuf), "OK");
      }
      else
      {
        responseLen = snprintf(responseBuf, sizeof(responseBuf), "ERROR");
      }
      break;

    case CMD_MOVE_ARM_JOINT:

      if (intArgCount == 3)
      {
        arm.moveServo(intArgs[0], intArgs[1], intArgs[2]);
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
