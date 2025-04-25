#include <Servo.h>

// === Servo Pins ===
#define SERVO_PIN_BASE 10    // Base
#define SERVO_PIN_SHOULDER 3 // Shoulder
#define SERVO_PIN_ELBOW 9    // Elbow
#define SERVO_PIN_WRIST 8    // Wrist
#define SERVO_PIN_GRIP 7     // Grip

#define GRIP_CLOSED 72
#define GRIP_OPEN 105

// === Default Angles ===
#define DEFAULT_ANGLE_BASE 90
#define DEFAULT_ANGLE_SHOULDER 90
#define DEFAULT_ANGLE_ELBOW 90
#define DEFAULT_ANGLE_WRIST 90
#define DEFAULT_ANGLE_GRIP 90

class Arm
{

private:
  Servo servoBase, servoShoulder, servoElbow, servoWrist, servoGrip;

  // Current angles
  int state_angle_base, state_angle_shoulder, state_angle_elbow, state_angle_wrist, state_angle_grip;

  void rest_and_print()
  {
    printMenu();
    delay(500);
  }

public:
  Arm()
  {
    state_angle_base = DEFAULT_ANGLE_BASE;
    state_angle_shoulder = DEFAULT_ANGLE_SHOULDER;
    state_angle_elbow = DEFAULT_ANGLE_ELBOW;
    state_angle_wrist = DEFAULT_ANGLE_WRIST;
    state_angle_grip = DEFAULT_ANGLE_GRIP;
  }

  void printMenu()
  {
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
  }

  void attachAll()
  {
    servoBase.attach(SERVO_PIN_BASE);
    servoShoulder.attach(SERVO_PIN_SHOULDER);
    servoElbow.attach(SERVO_PIN_ELBOW);
    servoWrist.attach(SERVO_PIN_WRIST, 500, 2400);
    servoGrip.attach(SERVO_PIN_GRIP, 500, 2400); // 1000 - 2000 ?? REVIEW
  }

  void moveServo(char motor, int requiredAngle, int overShoot)
  {
    // Clamp angle to safe range
    requiredAngle = constrain(requiredAngle, 10, 170);

    // Motor-specific settings
    int baseDelay = 40, slowDelay = 80, threshold = 0;
    const char *motorName = "";
    Servo s;
    int stateAngle;

    switch (motor)
    {
    case 'b':
      threshold = 20;
      motorName = "base";
      s = servoBase;
      stateAngle = state_angle_base;
      break;
    case 's':
      threshold = 0;
      motorName = "shoulder";
      s = servoShoulder;
      stateAngle = state_angle_shoulder;
      break;
    case 'e':
      threshold = 5;
      motorName = "elbow";
      s = servoElbow;
      stateAngle = state_angle_elbow;
      break;
    case 'w':
      threshold = 0;
      motorName = "wrist";
      s = servoWrist;
      stateAngle = state_angle_wrist;
      break;
    case 'g':
      threshold = 0;
      motorName = "grip";
      s = servoGrip;
      stateAngle = state_angle_grip;
      break;
    default:
      return;
    }

    if (requiredAngle == stateAngle)
    {
      Serial.print(motorName);
      Serial.print(" is already on angle: ");
      Serial.println(requiredAngle);
      return;
    }

    // Move to target angle
    int step = (requiredAngle > stateAngle) ? 1 : -1;
    for (int i = stateAngle; i != requiredAngle + step; i += step)
    {
      int remaining = abs(requiredAngle - i);
      int delayMs = (remaining > threshold) ? baseDelay : slowDelay;
      s.write(i);
      delay(delayMs);
    }

    // Applying overshoot
    if (overShoot > 0)
    {
      for (int i = requiredAngle; i <= requiredAngle + overShoot; i++)
      {
        s.write(i);
        delay(baseDelay);
      }
      for (int i = requiredAngle + overShoot; i >= requiredAngle; i--)
      {
        s.write(i);
        delay(baseDelay);
      }
    }

    // Update state and print if angle changed
    switch (motor)
    {
    case 'b':
      state_angle_base = requiredAngle;
      break;
    case 's':
      state_angle_shoulder = requiredAngle;
      break;
    case 'e':
      state_angle_elbow = requiredAngle;
      break;
    case 'w':
      state_angle_wrist = requiredAngle;
      break;
    case 'g':
      state_angle_grip = requiredAngle;
      break;
    default:
      return;
    }

    Serial.print("Moved ");
    Serial.println(motorName);

    rest_and_print();
  }
};
