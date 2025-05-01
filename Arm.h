#include <Servo.h>
#include "DEFs.h"

enum ArmMotor
{
  BASE = 0,
  SHOULDER = 1,
  ELBOW = 2,
  WRIST = 3,
  GRIP = 4
};

class Arm
{
private:
  Servo servoBase, servoShoulder, servoElbow, servoWrist, servoGrip;

  // Current angles
  int state_angle_base, state_angle_shoulder, state_angle_elbow, state_angle_wrist, state_angle_grip;

  void printMessage(const char *msg, bool newLine = false)
  {
#if ENABLE_DEBUG
    if (newLine)
    {
      Serial.println(msg);
    }
    else
    {
      Serial.print(msg);
    }
#endif
  }

  void printMenu()
  {
#if ENABLE_DEBUG
    Serial.print("B: ");
    Serial.println(state_angle_base);
    Serial.print("S: ");
    Serial.println(state_angle_shoulder);
    Serial.print("E: ");
    Serial.println(state_angle_elbow);
    Serial.print("W: ");
    Serial.println(state_angle_wrist);
    Serial.print("G: ");
    Serial.println(state_angle_grip);
    Serial.println();
#endif
  }

  void attachAll()
  {
    servoBase.attach(SERVO_PIN_BASE);
    servoShoulder.attach(SERVO_PIN_SHOULDER);
    servoElbow.attach(SERVO_PIN_ELBOW);
    servoWrist.attach(SERVO_PIN_WRIST, 500, 2400);
    servoGrip.attach(SERVO_PIN_GRIP, 500, 2400); // 1000 - 2000 ?? REVIEW
  }

public:
  Arm()
  {
  }

  void initializeArm()
  {

    state_angle_base = DEFAULT_ANGLE_BASE;
    state_angle_shoulder = DEFAULT_ANGLE_SHOULDER;
    state_angle_elbow = DEFAULT_ANGLE_ELBOW;
    state_angle_wrist = DEFAULT_ANGLE_WRIST;
    state_angle_grip = DEFAULT_ANGLE_GRIP;
    attachAll();
    moveServo(BASE, DEFAULT_ANGLE_BASE, 0);
    moveServo(SHOULDER, DEFAULT_ANGLE_SHOULDER, 4);
    moveServo(ELBOW, DEFAULT_ANGLE_ELBOW, 0);
    moveServo(WRIST, DEFAULT_ANGLE_WRIST, 4);
    moveServo(GRIP, DEFAULT_ANGLE_GRIP, 0);

    printMessage("Arm Motors intialized", true);
    printMenu();
    delay(500);
  }

  void moveServo(ArmMotor motor, int requiredAngle, int overShoot)
  {
    // Clamp angle to safe range
    requiredAngle = constrain(requiredAngle, 10, 170);

    // Motor-specific settings
    int baseDelay = 40, slowDelay = 80, threshold = 0;
    const char *motorName = "";
    Servo *s = nullptr;
    int *stateAngle = nullptr;

    switch (motor)
    {
    case BASE:
      threshold = 20;
      motorName = "base";
      s = &servoBase;
      stateAngle = &state_angle_base;
      break;
    case SHOULDER:
      threshold = 0;
      motorName = "shoulder";
      s = &servoShoulder;
      stateAngle = &state_angle_shoulder;
      break;
    case ELBOW:
      threshold = 5;
      motorName = "elbow";
      s = &servoElbow;
      stateAngle = &state_angle_elbow;
      break;
    case WRIST:
      threshold = 0;
      motorName = "wrist";
      s = &servoWrist;
      stateAngle = &state_angle_wrist;
      break;
    case GRIP:
      threshold = 0;
      motorName = "grip";
      s = &servoGrip;
      stateAngle = &state_angle_grip;
      break;
    default:
      return;
    }

    if (requiredAngle == *stateAngle)
    {
      printMessage(motorName);
      printMessage(" is already on angle: ");
      printMessage(requiredAngle, true);
      return;
    }

    // Move to target angle
    int step = (requiredAngle > *stateAngle) ? 1 : -1;
    for (int i = *stateAngle; i != requiredAngle + step; i += step)
    {
      int remaining = abs(requiredAngle - i);
      int delayMs = (remaining > threshold) ? baseDelay : slowDelay;
      s->write(i);
      delay(delayMs);
    }

    // Applying overshoot
    if (overShoot > 0)
    {
      for (int i = requiredAngle; i <= requiredAngle + overShoot; i++)
      {
        s->write(i);
        delay(baseDelay);
      }
      for (int i = requiredAngle + overShoot; i >= requiredAngle; i--)
      {
        s->write(i);
        delay(baseDelay);
      }
    }

    // Update state and print if angle changed
    *stateAngle = requiredAngle;

    printMessage("Moved: ");
    printMessage(motorName, true);

    printMenu();
    delay(500);
  }

  void moveArmTo(int base, int shoulder, int elbow, int wrist, int grip)
  {
    // to be implemented
  }
};
