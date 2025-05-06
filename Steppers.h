#include <Arduino.h>
#include "DEFs.h"

class Steppers
{
public:
  Steppers()
  {
  }

  void initialize()
  {
    // Initialize stepper pins
    assignPins();
  }

  /**
   * Received array indicates the angle, direction of the motor to be moved
   * vals[i] != 0 means motor i should be moved
   * With vals[i] degree, and in vals[i+1] direction
   */
  void runCommand(int vals[STEPPER_COUNT])
  {
    if (vals[0] != 0)
    {
      rotateMotor(vals[0], vals[1], STEP_PIN_1, DIR_PIN_1);
    }
    if (vals[2] != 0)
    {
      rotateMotor(vals[2], vals[3], STEP_PIN_2, DIR_PIN_2);
    }
    if (vals[4] != 0)
    {
      rotateMotor(vals[4], vals[5], STEP_PIN_3, DIR_PIN_3);
    }
    if (vals[6] != 0)
    {
      rotateMotor(vals[6], vals[7], STEP_PIN_4, DIR_PIN_4);
    }
    if (vals[8] != 0)
    {
      rotateMotor(vals[8], vals[9], STEP_PIN_5, DIR_PIN_5);
    }
  }

  void rotateMotor(int angle, int direction, int stepPin, int dirPin)
  {
    digitalWrite(dirPin, direction == 1 ? HIGH : LOW); // Set direction
    int steps = angle / 1.8;
    for (int i = 0; i < steps; i++)
    {
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(1000);
      digitalWrite(stepPin, LOW);
      delayMicroseconds(1000);
    }
  }

  void assignPins()
  {
    pinMode(STEP_PIN_1, OUTPUT);
    pinMode(DIR_PIN_1, OUTPUT);

    pinMode(STEP_PIN_2, OUTPUT);
    pinMode(DIR_PIN_2, OUTPUT);

    pinMode(STEP_PIN_3, OUTPUT);
    pinMode(DIR_PIN_3, OUTPUT);

    pinMode(STEP_PIN_4, OUTPUT);
    pinMode(DIR_PIN_4, OUTPUT);

    pinMode(STEP_PIN_5, OUTPUT);
    pinMode(DIR_PIN_5, OUTPUT);
  }
};
