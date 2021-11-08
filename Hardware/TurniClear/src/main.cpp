/*
    MPU6050 Triple Axis Gyroscope & Accelerometer. Pitch & Roll & Yaw Gyroscope Example.
    Read more: http://www.jarzebski.pl/arduino/czujniki-i-sensory/3-osiowy-zyroskop-i-akcelerometr-mpu6050.html
    GIT: https://github.com/jarzebski/Arduino-MPU6050
    Web: http://www.jarzebski.pl
    (c) 2014 by Korneliusz Jarzebski
*/

#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

#define LEFT        2
#define RIGHT       3

#define LEFTLOAD    4
#define RIGHTLOAD   5



// Timers
unsigned long timer = 0;
float timeStep = 0.01;

long previousMillis = 0;
unsigned long interval = 500; 

float yaw = 0;
bool isBlinking = false;
bool blinkState = LOW;

enum turns {
  left = LEFTLOAD,
  right = RIGHTLOAD,
  none
};

turns ledBlinking = none;


void cancelTurnSignals()
{
  digitalWrite(ledBlinking, LOW);
  isBlinking = false;
  ledBlinking = none;
}


void activateTurnSignal(turns dir){
  if(isBlinking){
      cancelTurnSignals();
      delay(250);
  } else {
      isBlinking = true;
      ledBlinking = dir;
      delay(250);
  }
}

void activateLeftSignal(){
    activateTurnSignal(left);
}

void activateRightSignal(){
    activateTurnSignal(right);
}

void setup() 
{
  Serial.begin(115200);

  pinMode(LEFT, INPUT_PULLUP);
  pinMode(RIGHT, INPUT_PULLUP);

  pinMode(LEFTLOAD, OUTPUT);
  pinMode(RIGHTLOAD, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(LEFT), activateLeftSignal, FALLING);
  attachInterrupt(digitalPinToInterrupt(RIGHT), activateRightSignal, FALLING);

  // Initialize MPU6050
  while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))
  {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }
  mpu.calibrateGyro();
  mpu.setThreshold(3);
}

void loop()
{ 
  timer = millis();

  Vector norm = mpu.readNormalizeGyro();

  yaw = yaw + norm.ZAxis * timeStep;

  Serial.print(" Yaw = ");
  Serial.println(yaw);

  // Wait to full timeStep period
  delay((timeStep*1000) - (millis() - timer));

  unsigned long currentMillis = millis();
 
  if(isBlinking && currentMillis - previousMillis > interval) {
    previousMillis = currentMillis; 
    digitalWrite(ledBlinking, blinkState);
    blinkState = !blinkState;
    Serial.println("glow");
  }
}