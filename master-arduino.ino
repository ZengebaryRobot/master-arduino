#include "DEFs.h"
#include "Arm.h"
#include "Steppers.h"

#if ENABLE_DEBUG
#include <NeoSWSerial.h>
#endif

HardwareSerial &espSerial = Serial;

Arm arm;
Steppers steppers;

#if ENABLE_DEBUG
NeoSWSerial debugSerial(10, 11); // 10 (RX) & 11 (TX)
#endif

void setup()
{
  espSerial.begin(9600);
  delay(200);

  arm.initializeArm();
  steppers.initialize();

#if ENABLE_DEBUG
  debugSerial.begin(9600);
  delay(200);

  debugSerial.println("Arduino ready!");
#endif
}

void loop()
{
  if (!espSerial.available())
    return;

  String line = espSerial.readStringUntil('\n');
  if (line.length() < 2)
    return;

#if ENABLE_DEBUG
  debugSerial.println("Received: " + line);
#endif

  char cmd = line.charAt(0);
  if (cmd == 'A' && line.charAt(1) == ',')
  {
    int a1, a2, a3;
    int parsed = sscanf(line.c_str() + 2, "%d,%d,%d", &a1, &a2, &a3);

#if ENABLE_DEBUG
    debugSerial.println("Servo command parsed: " + String(a1) + ", " + String(a2) + ", " + String(a3));
#endif

    if (parsed == 3)
    {
      arm.moveServo(a1, a2, a3);
      espSerial.println("OK");

#if ENABLE_DEBUG
      debugSerial.println("Servo command success: " + String(a1) + ", " + String(a2) + ", " + String(a3));
#endif
    }
    else
    {
      espSerial.println("ERROR");

#if ENABLE_DEBUG
      debugSerial.println("Servo command error: " + line);
#endif
    }
  }
  else if (cmd == 'S' && line.charAt(1) == ',')
  {
    int vals[STEPPER_COUNT];
    int parsed = sscanf(line.c_str() + 2, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", &vals[0], &vals[1], &vals[2], &vals[3], &vals[4], &vals[5], &vals[6], &vals[7], &vals[8], &vals[9]);

#if ENABLE_DEBUG
    debugSerial.print("Steppers command parsed: ");
    for (int i = 0; i < STEPPER_COUNT; i++)
    {
      debugSerial.print(vals[i]);
      if (i < STEPPER_COUNT - 1)
        debugSerial.print(", ");
    }
    debugSerial.println();
#endif

    if (parsed == STEPPER_COUNT)
    {
      steppers.runCommand(vals);
      espSerial.println("OK");

#if ENABLE_DEBUG
      debugSerial.print("Steppers command success");
#endif
    }
    else
    {
      espSerial.println("ERROR");
#if ENABLE_DEBUG
      debugSerial.print("Steppers command error");
#endif
    }
  }

  while (espSerial.available())
    espSerial.read();
}
