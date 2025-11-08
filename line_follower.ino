// ================= QTR-8RC Line Follower — PD + centroid (BLACK LINE) =================
// Pin map avoids conflict between QTR ch #6 and ENB=6 by moving that channel to A2 (pin 16).
// Tested on Arduino Uno + L298N. Uses QTR-8RC (RC-timed) array.
//
// Serial tweaks (send single chars):
//   S  -> show params
//   + / -  -> Kp up/down by 0.02
//   > / <  -> Kd up/down by 0.05
//   ] / [  -> BASE_SPEED up/down by 5
//   ) / (  -> BOOST up/down by 2
//
// Tip: set sensor height ~2–3 mm above mat; add a small light shroud for best results.

#include <QTRSensors.h>

// ---------------- Motor Pins (L298N) ----------------
const int ENA = 10;   // Right PWM
const int IN1 = 11;
const int IN2 = 12;
const int IN3 = 13;
const int IN4 = 9;
const int ENB = 6;    // Left PWM  (kept on D6)

// ---------------- QTR-8RC Setup ---------------------
// QTR OUT1..OUT8 -> {2,3,4,5,7,A2(16),8,A0(14)}
// A1(15) used as emitter control (optional; safe if not wired)
const uint8_t qtrPins[8] = {2,3,4,5,7,16,8,14};
const int QTR_EMITTER_PIN = 15;    // A1
QTRSensors qtr;

uint16_t qtrValues[8];             // filled by readLineBlack()

// ---------------- Speeds & Control ------------------
int BASE_SPEED = 90;     // cruise PWM (0..255)
int MIN_PWM    = 60;     // ensure motors overcome friction & L298N drop
int BOOST      = 20;     // extra speed on straights

// PD gains (start here, tune on mat)
float Kp = 0.22f;        // proportional on position error (counts)
float Kd = 1.10f;        // derivative on error delta

// Limits & helpers
int lastDirection = 0;   // 0 center, 1 left, 2 right
unsigned long lostSince = 0;

// ---------------- Motor helpers ---------------------
void setLR(int leftPWM, int rightPWM) {
  leftPWM  = constrain(leftPWM,  0, 255);
  rightPWM = constrain(rightPWM, 0, 255);

  // forward
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);

  if (leftPWM  > 0 && leftPWM  < MIN_PWM) leftPWM  = MIN_PWM;
  if (rightPWM > 0 && rightPWM < MIN_PWM) rightPWM = MIN_PWM;

  analogWrite(ENB, leftPWM);
  analogWrite(ENA, rightPWM);
}

void stopMotors() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  analogWrite(ENA, 0); analogWrite(ENB, 0);
}

void spinLeft(int pwm) {
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH); // right backward
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);  // left forward
  analogWrite(ENA, pwm);
  analogWrite(ENB, pwm);
}

void spinRight(int pwm) {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);  // right forward
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH); // left backward
  analogWrite(ENA, pwm);
  analogWrite(ENB, pwm);
}

// ---------------- Setup -----------------------------
void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);

  qtr.setTypeRC();
  qtr.setSensorPins(qtrPins, 8);
  qtr.setEmitterPin(QTR_EMITTER_PIN); // harmless if not connected

  // Brief calibration: hold bot and gently sweep over black/white
  delay(250);
  for (int i = 0; i < 120; i++) {  // ~600 ms
    qtr.calibrate();
    delay(5);
  }

  Serial.println(F("QTR-8RC LFR Ready (BLACK line). Send 'S' to show params."));
}

// ---------------- Loop ------------------------------
void loop() {
  // ---- serial live tweaks ----
  if (Serial.available()) {
    char c = toupper(Serial.read());
    if (c == 'S') {
      Serial.print(F("Kp=")); Serial.print(Kp, 3);
      Serial.print(F("  Kd=")); Serial.print(Kd, 3);
      Serial.print(F("  BASE=")); Serial.print(BASE_SPEED);
      Serial.print(F("  BOOST=")); Serial.print(BOOST);
      Serial.print(F("  MIN=")); Serial.println(MIN_PWM);
    } else if (c == '+') Kp += 0.02f;
    else if (c == '-')   Kp -= 0.02f;
    else if (c == '>')   Kd += 0.05f;
    else if (c == '<')   Kd -= 0.05f;
    else if (c == ']')   BASE_SPEED = min(255, BASE_SPEED + 5);
    else if (c == '[')   BASE_SPEED = max(0,   BASE_SPEED - 5);
    else if (c == ')')   BOOST = min(80, BOOST + 2);
    else if (c == '(')   BOOST = max(0,  BOOST - 2);
  }

  // Read BLACK line: pos in [0..7000], center ≈ 3500
  uint16_t pos = qtr.readLineBlack(qtrValues);

  // Simple lost detection: very low overall darkness -> likely on white
  long total = 0;
  for (int i = 0; i < 8; i++) total += qtrValues[i];
  if (total < 300) { // adjust by mat/lighting; 300–600 typical
    if (lostSince == 0) lostSince = millis();
    uint32_t t = millis() - lostSince;

    if (t < 120) {
      setLR(BASE_SPEED, BASE_SPEED);               // tiny coast forward
    } else if (t < 450) {
      if (lastDirection == 1) setLR(BASE_SPEED*0.6, BASE_SPEED);  // arc left
      else                    setLR(BASE_SPEED, BASE_SPEED*0.6);  // arc right
    } else {
      if (lastDirection == 1) spinLeft(80); else spinRight(80);   // in-place spin
    }
    return;
  }
  lostSince = 0;

  // PD control on position error
  static long lastError = 0;
  long error = (long)pos - 3500;     // negative = line left, positive = right
  long derr  = error - lastError;
  float steer = Kp * error + Kd * derr;
  lastError = error;

  // clamp steer to a sane range (affects turn intensity)
  if (steer > 120) steer = 120;
  if (steer < -120) steer = -120;

  // Speed shaping: add boost when essentially straight
  int boost = (abs(error) < 300 && fabs(steer) < 25) ? BOOST : 0;
  int base  = constrain(BASE_SPEED + boost, 0, 255);

  int left  = base - (int)steer;
  int right = base + (int)steer;

  setLR(left, right);

  // remember recent heading bias for recovery logic
  if (error < -120)      lastDirection = 1;
  else if (error > 120)  lastDirection = 2;
  else                   lastDirection = 0;
}
  