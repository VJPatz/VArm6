/**
 * @file    main.cpp
 * @brief   6-DOF Robotic Arm — Joystick Controller
 * @author  [VJPatz]
 * @version 1.0.0
 *
 * Controls a 6-joint robotic arm via three analog joysticks.
 * Each joystick handles two axes, with diagonal input blocked
 * to prevent unintended multi-joint movement.
 *
 * Hardware:
 *  - Arduino Mega 2560
 *  - PCA9685 PWM Servo Driver (I2C, address 0x40)
 *  - 6× hobby servos (SG90 / MG996R or equivalent)
 *  - 3× dual-axis analog joysticks (with one clickable switch)
 *
 * Wiring summary:
 *  - Joystick 1 X/Y → A8/A9   (Base / Shoulder)
 *  - Joystick 2 X/Y → A10/A11 (Elbow / Wrist)
 *  - Joystick 3 X/Y → A12/A13 (Rotate / Grip)
 *  - Joystick 1 button → D18 (hold = return to home position)
 *
 * PCA9685 connections:
 *  - SDA → Pin 20, SCL → Pin 21 (hardware I2C on Mega)
 *  - Channel 0–5 → Base, Shoulder, Elbow, Wrist, Rotate, Grip
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// ─────────────────────────────────────────────
//  PCA9685 Driver (default I2C address 0x40)
// ─────────────────────────────────────────────
Adafruit_PWMServoDriver pca = Adafruit_PWMServoDriver(0x40);

// ─────────────────────────────────────────────
//  Servo Channel Assignments (PCA9685 outputs)
// ─────────────────────────────────────────────
#define BASE     0   // Rotates the arm left/right
#define SHOULDER 1   // Lifts/lowers upper arm
#define ELBOW    2   // Bends at mid-arm joint
#define WRIST    3   // Tilts the end-effector
#define ROTATE   4   // Spins the wrist/tool
#define GRIP     5   // Opens/closes the gripper

// ─────────────────────────────────────────────
//  Joystick Analog Pins (Arduino Mega)
// ─────────────────────────────────────────────
#define J1X A8    // Joystick 1 horizontal → BASE
#define J1Y A9    // Joystick 1 vertical   → SHOULDER
#define J2X A10   // Joystick 2 horizontal → ELBOW
#define J2Y A11   // Joystick 2 vertical   → WRIST
#define J3X A12   // Joystick 3 horizontal → ROTATE
#define J3Y A13   // Joystick 3 vertical   → GRIP

// Joystick 1 button — hold to glide back to home position
#define J1SW 18

// ─────────────────────────────────────────────
//  Home Position (degrees)
//  Safe neutral pose the arm returns to on button hold.
//  Adjust to match your physical arm's resting position.
// ─────────────────────────────────────────────
int homeAngle[6] = { 90, 160, 120, 90, 90, 70 };

// Current position of each servo (starts at home)
int servoPos[6]  = { 90, 160, 120, 90, 90, 70 };

// ─────────────────────────────────────────────
//  Joint Travel Limits (degrees)
//  Tune these to match your physical arm's safe range
//  and avoid hitting mechanical hard stops.
// ─────────────────────────────────────────────
int minAngle[6] = {
   20,  // Base     — leftmost sweep limit
  120,  // Shoulder — lowest safe lift angle
   90,  // Elbow    — maximum fold
   40,  // Wrist    — downward tilt limit
   40,  // Rotate   — counterclockwise limit
   20   // Grip     — fully open
};

int maxAngle[6] = {
  160,  // Base     — rightmost sweep limit
  160,  // Shoulder — fully raised
  140,  // Elbow    — fully extended
  140,  // Wrist    — maximum upward tilt
  140,  // Rotate   — clockwise limit
  120   // Grip     — fully closed
};

// ─────────────────────────────────────────────
//  PCA9685 Pulse Width Range (μs counts)
//  120 ≈ 0.6 ms (0°), 600 ≈ 2.4 ms (180°)
//  Adjust if your servos don't reach full range cleanly.
// ─────────────────────────────────────────────
#define SERVOMIN 120
#define SERVOMAX 600

// ─────────────────────────────────────────────
//  Joystick Dead-Zone Thresholds (0–1023 ADC)
//  Values outside [LOW_TH, HIGH_TH] register as input.
//  Widen these if you experience joystick drift at rest.
// ─────────────────────────────────────────────
#define LOW_TH  300
#define HIGH_TH 750

// ─────────────────────────────────────────────
//  Motion Tuning
//  UPDATE_INTERVAL — how often positions are refreshed (ms)
//    Lower = faster response; higher = smoother / slower
//  MAX_SPEED — maximum degrees moved per update tick
//  ACCEL / DECEL — how quickly speed ramps up or coasts down
// ─────────────────────────────────────────────
#define UPDATE_INTERVAL 25
#define MAX_SPEED        2
#define ACCEL            1
#define DECEL            1

// Live speed per joint (degrees/tick, signed)
int speed[6]           = { 0, 0, 0, 0, 0, 0 };
unsigned long lastUpdate = 0;

// ─────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────

/** Convert an angle in degrees to a PCA9685 PWM count. */
int angleToPulse(int angle) {
  return map(angle, 0, 180, SERVOMIN, SERVOMAX);
}

/** Push the current position for one channel to the PCA9685. */
void writeServo(byte ch) {
  pca.setPWM(ch, 0, angleToPulse(servoPos[ch]));
}

/**
 * Advance one joint by its current speed, clamp to limits,
 * then write to hardware.
 */
void applyMotion(byte ch) {
  servoPos[ch] += speed[ch];
  servoPos[ch]  = constrain(servoPos[ch], minAngle[ch], maxAngle[ch]);
  writeServo(ch);
}

// ─────────────────────────────────────────────
//  Home Routine
//  Holds all joints while the button is pressed,
//  nudging each one degree at a time toward its home angle.
// ─────────────────────────────────────────────
void homeWhileHeld() {
  Serial.println("[Home] Button held — gliding to home position");

  // Zero out any in-flight momentum
  for (byte i = 0; i < 6; i++) speed[i] = 0;

  while (digitalRead(J1SW) == LOW) {
    for (byte i = 0; i < 6; i++) {
      if      (servoPos[i] < homeAngle[i]) servoPos[i]++;
      else if (servoPos[i] > homeAngle[i]) servoPos[i]--;
      writeServo(i);
    }
    delay(30);  // Slow enough to be visually smooth
  }

  Serial.println("[Home] Released — resuming manual control");
}

// ─────────────────────────────────────────────
//  Setup
// ─────────────────────────────────────────────
void setup() {
  Serial.begin(9600);
  Wire.begin();  // Mega: SDA = pin 20, SCL = pin 21

  pinMode(J1SW, INPUT_PULLUP);

  // Verify PCA9685 is reachable before continuing
  Wire.beginTransmission(0x40);
  if (Wire.endTransmission() == 0) {
    Serial.println("[Init] PCA9685 detected — OK");
  } else {
    Serial.println("[Init] ERROR: PCA9685 not found. Check wiring and I2C address.");
    while (1);  // Halt — nothing works without the servo driver
  }

  pca.begin();
  pca.setPWMFreq(50);  // 50 Hz — standard for hobby servos
  delay(10);

  // Drive all servos to home position on power-up
  for (byte i = 0; i < 6; i++) writeServo(i);

  Serial.println("[Init] All servos at home. Ready.");
}

// ─────────────────────────────────────────────
//  Main Loop
// ─────────────────────────────────────────────
void loop() {

  // Home button overrides everything else
  if (digitalRead(J1SW) == LOW) {
    homeWhileHeld();
    return;
  }

  // Rate-limit updates to UPDATE_INTERVAL ms
  if (millis() - lastUpdate < UPDATE_INTERVAL) return;
  lastUpdate = millis();

  // Sample all joystick axes
  int j1x = analogRead(J1X);   // BASE
  int j1y = analogRead(J1Y);   // SHOULDER
  int j2x = analogRead(J2X);   // ELBOW
  int j2y = analogRead(J2Y);   // WRIST
  int j3x = analogRead(J3X);   // ROTATE
  int j3y = analogRead(J3Y);   // GRIP

  // Tracks whether any axis is actively deflected this tick
  bool active = false;

  // ── Joystick 1: BASE (X) / SHOULDER (Y) ─────────────────
  // Only one axis acts at a time — diagonal input is ignored.
  // This prevents simultaneous joint movement from a sloppy release.
  bool j1xActive = (j1x < LOW_TH || j1x > HIGH_TH);
  bool j1yActive = (j1y < LOW_TH || j1y > HIGH_TH);

  if (j1xActive && !j1yActive) {
    if      (j1x < LOW_TH)  { speed[BASE] -= ACCEL; active = true; }
    else if (j1x > HIGH_TH) { speed[BASE] += ACCEL; active = true; }
  }
  else if (j1yActive && !j1xActive) {
    if      (j1y < LOW_TH)  { speed[SHOULDER] -= ACCEL; active = true; }
    else if (j1y > HIGH_TH) { speed[SHOULDER] += ACCEL; active = true; }
  }

  // ── Joystick 2: ELBOW (X) / WRIST (Y) ───────────────────
  bool j2xActive = (j2x < LOW_TH || j2x > HIGH_TH);
  bool j2yActive = (j2y < LOW_TH || j2y > HIGH_TH);

  if (j2xActive && !j2yActive) {
    if      (j2x < LOW_TH)  { speed[ELBOW] -= ACCEL; active = true; }
    else if (j2x > HIGH_TH) { speed[ELBOW] += ACCEL; active = true; }
  }
  else if (j2yActive && !j2xActive) {
    if      (j2y < LOW_TH)  { speed[WRIST] -= ACCEL; active = true; }
    else if (j2y > HIGH_TH) { speed[WRIST] += ACCEL; active = true; }
  }

  // ── Joystick 3: ROTATE (X) / GRIP (Y) ───────────────────
  bool j3xActive = (j3x < LOW_TH || j3x > HIGH_TH);
  bool j3yActive = (j3y < LOW_TH || j3y > HIGH_TH);

  if (j3xActive && !j3yActive) {
    if      (j3x < LOW_TH)  { speed[ROTATE] -= ACCEL; active = true; }
    else if (j3x > HIGH_TH) { speed[ROTATE] += ACCEL; active = true; }
  }
  else if (j3yActive && !j3xActive) {
    if      (j3y < HIGH_TH) { speed[GRIP] += ACCEL; active = true; }
    else if (j3y > LOW_TH)  { speed[GRIP] -= ACCEL; active = true; }
  }

  // Clamp all speeds within allowed range
  for (byte i = 0; i < 6; i++) {
    speed[i] = constrain(speed[i], -MAX_SPEED, MAX_SPEED);
  }

  // Coast to a stop when joystick is centered or diagonal
  if (!active) {
    for (byte i = 0; i < 6; i++) {
      if (speed[i] > 0) speed[i] -= DECEL;
      if (speed[i] < 0) speed[i] += DECEL;
    }
  }

  // Push updated positions to all servos
  applyMotion(BASE);
  applyMotion(SHOULDER);
  applyMotion(ELBOW);
  applyMotion(WRIST);
  applyMotion(ROTATE);
  applyMotion(GRIP);
}
