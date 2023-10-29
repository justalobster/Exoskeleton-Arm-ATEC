#include <Arduino.h>
#include "HX711.h"
//===========================================
// HX711 circuit wiring
#define btn1 11
#define DEBOUNCE_DELAY 100
#define DELAY_ON_TOP 3000
const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN = 3;
const int bientro = A0;
const int pos_min = 50;
const int pos_max = 980;
const int speed_min = 20;
const int speed_max = 255;
const int RPWM = 5;
const int LPWM = 6;
const HX711 scale;
bool emergency = false;
bool motorRunning = false;
bool isUp = false;
bool isDown = false;
bool isOnTop = false;
int btn1State = LOW;
float force = 0.0;
//===========================================
int lastBtn1State = LOW;
unsigned long lastDebounceTime = 0;
const unsigned long longPressDuration = 2000;
unsigned long lastLightToggle = 0;
//===========================================
void setup() {
  Serial.begin(9600);
  pinMode(btn1, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  attachInterrupt(digitalPinToInterrupt(btn1), handleISR, FALLING);
  Serial.println("HX711 Demo");
  Serial.println("Initializing the scale");

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  scale.set_scale(-459.542);
  scale.tare();  // reset the scale to 0

  Serial.println("Readings:");

  //////////////////////////
  pinMode(RPWM, OUTPUT);
  pinMode(LPWM, OUTPUT);
  Motor_Control(0, 0);
}
//===========================================
void loop() {
  readLoadCell();
  Serial.print(emergency);
  Serial.print("\t");
  Serial.print(motorRunning);
  Serial.print("\t");
  if (emergency && motorRunning) {
    Serial.println("Emergency stop");
    delay(1000);
    emergency = false;
    motorRunning = false;
    return;
  } else {
    int value_bientro = analogRead(A0);
    Serial.print(value_bientro);
    Serial.print("\t");
    int speed = 0;
    if (!isDown) {
      speed = speed_max*0.7;
    } else {
      speed = speed_max*0.8;
    }
    if (force < -100 && abs(value_bientro - pos_max) < 40) {
      motorRunning = true;
      isUp = true;
    }
    if (isUp) {
      Serial.println("Running Up");
      Serial.print("\t");
      Serial.print(speed);
      Motor_Control(speed, 0);  // moving up
    }

    if (abs(value_bientro - pos_min) < 50 && isUp == true) {
      motorRunning = false;
      Serial.println("Stop in Up");
      isUp = false;
      isOnTop = true;
    }

    if (isOnTop) {
      Motor_Control(0, 0);  // stop
      delay(DELAY_ON_TOP);
      isDown = true;
      isOnTop = false;
    }

    if (isDown) {
      motorRunning == true;
      Motor_Control(0, speed);
      Serial.print("\t");
      Serial.print(speed);
      Serial.print("\t");
      Serial.print("Running down");
    }

    if (isDown && abs(value_bientro - pos_max) < 40) {
      isDown = false;
      Serial.println("reach bottom");
      Motor_Control(0, 0);  // stop
      motorRunning = false;
    }
  }
  Serial.println();
}

void readLoadCell() {
  Serial.print("Reading: ");
  force = scale.get_units();
  Serial.print(force);
  Serial.print(" kg\t");
}

//===========================================
void Motor_Control(int L_speed, int R_speed) {
  analogWrite(RPWM, R_speed);
  analogWrite(LPWM, L_speed);
}
//===========================================
void handleISR() {
  if (millis() - lastDebounceTime > DEBOUNCE_DELAY && motorRunning) {
    Motor_Control(0, 0);          // stop motor in case of emergency
    emergency = true;             // set emergency flag
    lastDebounceTime = millis();  // reset the debounce timer
  }
}
//===========================================
void diLenLight() {
  // if ()
  digitalWrite(13, !digitalRead(13));
  delay(100);
}