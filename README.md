# 🛰️ Zengebary Master

## 📡 Master Arduino I2C Address: 0x10

## 📜 Command Types

When sending a command:

1. First byte = **Command Type**
2. Followed by **Command-Specific Arguments**

| Command              | Code (`uint8_t`) | Purpose                            |
| -------------------- | ---------------- | ---------------------------------- |
| `CMD_CAMERA`         | `1`              | Request vision data from ESP32-CAM |
| `CMD_DISPLAY`        | `2`              | Display a short text on LCD        |
| `CMD_MOVE_ARM`       | `3`              | Move the robotic arm               |
| `CMD_MOVE_ARM_JOINT` | `4`              | Move a specific arm joint          |

---

## 🛠 How to Send a Command

### General Flow:

1. Start I2C transmission to `0x10`
2. Send 1 byte: **Command Type**
3. Send **Command-Specific Data** (depends on command)
4. End transmission
5. After some delay (~10ms), request the **response**

---

## 🧵 Command Details

---

### 📷 1. CMD_CAMERA (Vision Request)

-   **Purpose**: Ask ESP32-CAM to detect an object or feature.
-   **Arguments**:
    -   A string (ASCII characters).
-   **Response**:
    -   Comma-separated integers (e.g., `"23,88,105"`)  
        OR
    -   `"ERROR"`

#### Example

**Write**:

```
[ 1 ][ 'x','o' ]
```

(no size prefix, just raw string)

**Then Read**:

```
"23,88,105"  or  "ERROR"
```

---

### 🖥️ 2. CMD_DISPLAY (LCD Text)

-   **Purpose**: Display a string on the 16x2 LCD.
-   **Arguments**:
    -   A string (max 32 characters).
-   **Response**:
    -   `"OK"` if successful
    -   `"ERROR"` if text too long (>32)

#### Example

**Write**:

```
[ 2 ][ 'H','e','l','l','o',' ','W','o','r','l','d' ]
```

**Then Read**:

```
"OK" or "ERROR"
```

---

### 🤖 3. CMD_MOVE_ARM (Move Robotic Arm)

-   **Purpose**: Move the robotic arm.
-   **Arguments**:
    -   5 bytes: Base, Shoulder, Elbow, Wrist, Grip angles (0–180).
-   **Response**:
    -   `"OK"` if 5 values received
    -   `"ERROR"` if not

#### Example

**Write**:

```
[ 3 ][ 90 ][ 45 ][ 120 ][ 60 ][ 10 ]
```

**Then Read**:

```
"OK" or "ERROR"
```

---

### 🦾 4. CMD_MOVE_ARM_JOINT (Move Specific Joint)

-   **Purpose**: Move a single joint of the robotic arm.
-   **Arguments**:
    -   3 bytes:
        -   Joint character ('b'=base, 's'=shoulder, 'e'=elbow, 'w'=wrist, 'g'=grip)
        -   Required angle (10-170)
        -   Overshoot value (helps overcome friction)
-   **Response**:
    -   `"OK"` if 3 values received
    -   `"ERROR"` if not

#### Example

**Write**:

```
[ 4 ][ 'b' ][ 90 ][ 5 ]
```

(Move base to 90° with 5° overshoot)

**Then Read**:

```
"OK" or "ERROR"
```

---

## 📜 Arduino Example Code — How to Communicate with the Master

### 🖥️ Example 1 — Send Display Text

```cpp
#include <Wire.h>

#define MASTER_ADDR 0x10

void setup() {
  Wire.begin();
  Serial.begin(9600);
  delay(1000);

  sendDisplayMessage("Hello World!");
}

void loop() {
  //
}

void sendDisplayMessage(const String &msg) {
  Wire.beginTransmission(MASTER_ADDR);
  Wire.write(2);
  for (size_t i = 0; i < msg.length(); i++) {
    Wire.write(msg[i]);
  }
  Wire.endTransmission();

  delay(20);

  requestAndPrintResponse();
}

void requestAndPrintResponse() {
  Wire.requestFrom(MASTER_ADDR, 32);

  String response = "";
  while (Wire.available()) {
    response += (char)Wire.read();
  }

  Serial.println("Response: " + response);
}
```

---

### 📷 Example 2 — Request Vision Data

```cpp
#include <Wire.h>

#define MASTER_ADDR 0x10

void setup() {
  Wire.begin();
  Serial.begin(9600);
  delay(1000);

  requestVision("xo");
}

void loop() {
  //
}

void requestVision(const String &cmd) {
  Wire.beginTransmission(MASTER_ADDR);
  Wire.write(1);
  for (size_t i = 0; i < cmd.length(); i++) {
    Wire.write(cmd[i]);
  }
  Wire.endTransmission();

  delay(50);

  Wire.requestFrom(MASTER_ADDR, 64);

  String data = "";
  while (Wire.available()) {
    data += (char)Wire.read();
  }

  Serial.println("Data: " + data);
}
```

---

### 🤖 Example 3 — Move the Robotic Arm

```cpp
#include <Wire.h>

#define MASTER_ADDR 0x10

void setup() {
  Wire.begin();
  Serial.begin(9600);
  delay(1000);

  moveArm(90, 45, 120, 60, 10);
}

void loop() {
  //
}

void moveArm(uint8_t base, uint8_t shoulder, uint8_t elbow, uint8_t wrist, uint8_t grip) {
  Wire.beginTransmission(MASTER_ADDR);
  Wire.write(3);
  Wire.write(base);
  Wire.write(shoulder);
  Wire.write(elbow);
  Wire.write(wrist);
  Wire.write(grip);
  Wire.endTransmission();

  delay(20);

  Wire.requestFrom(MASTER_ADDR, 32);

  String response = "";
  while (Wire.available()) {
    response += (char)Wire.read();
  }

  Serial.println("Response: " + response);
}
```

---

### 🦾 Example 4 — Move a Single Joint

```cpp
#include <Wire.h>

#define MASTER_ADDR 0x10

void setup() {
  Wire.begin();
  Serial.begin(9600);
  delay(1000);

  moveJoint('b', 90, 5);  // Move base to 90° with 5° overshoot
}

void loop() {
  //
}

void moveJoint(char joint, uint8_t angle, uint8_t overshoot) {
  Wire.beginTransmission(MASTER_ADDR);
  Wire.write(4);
  Wire.write(joint);
  Wire.write(angle);
  Wire.write(overshoot);
  Wire.endTransmission();

  delay(20);

  Wire.requestFrom(MASTER_ADDR, 32);

  String response = "";
  while (Wire.available()) {
    response += (char)Wire.read();
  }

  Serial.println("Response: " + response);
}
```

# 🛡️ Summary Table

| Command        | Format                                         | Response    |
| -------------- | ---------------------------------------------- | ----------- |
| Camera         | `[1] [ASCII chars]`                            | CSV / ERROR |
| Display        | `[2] [ASCII chars] (max 32)`                   | OK / ERROR  |
| Move Arm       | `[3] [base] [shoulder] [elbow] [wrist] [grip]` | OK / ERROR  |
| Move Arm Joint | `[4] [joint char] [angle] [overshoot]`         | OK / ERROR  |
