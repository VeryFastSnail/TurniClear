
#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

#define LEFT                  2
#define RIGHT                 3

#define LEFTLOAD              4
#define RIGHTLOAD             5

#define CANCELATION_ANGLE     10
#define TURN_QUALIFIER        45


//===========Buttons routine declarations===============//

unsigned long buttonTimer = 0;
unsigned long longPressTime = 500;

boolean buttonActive = false;
boolean longPressActive = false;

enum buttons {
  left = LEFTLOAD,
  right = RIGHTLOAD,
  none
};


//===========Turn Signals routine declarations============//
buttons ledBlinking = none;
#define SHORTBLINKTIMES         3
#define BLINKSAJUSTED           SHORTBLINKTIMES * 2
int blinks = SHORTBLINKTIMES; // because it goes on an off so double the itterations
bool shortBlinking = false;

int leftButtonState = 0;
int rightButtonState = 0;
bool isTurned = false;


// Timers
unsigned long timer = 0;
float timeStep = 0.01;

long previousMillis = 0;
unsigned long interval = 500; 

float yaw = 0;
bool isBlinking = false;
bool blinkState = LOW;


void cancelTurnSignals()
{
  shortBlinking = false;
  digitalWrite(ledBlinking, LOW);
  isBlinking = false;
  ledBlinking = none;
  leftButtonState = HIGH;
  rightButtonState = HIGH;
  isTurned = false;
  Serial.println("Canceled Turn signals");
}


void activateTurnSignal(buttons dir){
  if(!isBlinking) {
      mpu.calibrateGyro(10); //added for calibrating bf turn.
      yaw = 0;
      Serial.println("Calibrated MPU");
      isBlinking = true;
      ledBlinking = dir;
  } else {
      cancelTurnSignals();
      delay(250);
  } 
}

void setup() 
{
  Serial.begin(115200);

  pinMode(LEFT, INPUT_PULLUP);
  pinMode(RIGHT, INPUT_PULLUP);

  pinMode(LEFTLOAD, OUTPUT);
  pinMode(RIGHTLOAD, OUTPUT);

  while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))
  {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }
  mpu.calibrateGyro(10);
  mpu.setThreshold(3);
}

void activateShortBlinks(buttons dir){
  blinks = SHORTBLINKTIMES*2;
  isBlinking = true;
  ledBlinking = dir;
  shortBlinking = true;
}

void loop()
{ 

  if (digitalRead(LEFT) == LOW || digitalRead(RIGHT) ==  LOW) {
    leftButtonState = digitalRead(LEFT);
    rightButtonState = digitalRead(RIGHT);

		if (!buttonActive) {
			buttonActive = true;
			buttonTimer = millis();
		}
		if ((millis() - buttonTimer > longPressTime) && (longPressActive == false)) {
			longPressActive = true;
      if(leftButtonState == LOW){
        activateTurnSignal(left);
        Serial.println("LEFT long");
      }else if(rightButtonState == LOW){
        Serial.println("RIGHT long");
        activateTurnSignal(right);
      }
		}
	} else {
		if (buttonActive == true) {
			if (longPressActive == true) {
				longPressActive = false;
			} else {
        if(isBlinking){
          cancelTurnSignals();
        } else {
          if(leftButtonState == LOW){
          activateShortBlinks(left);
          Serial.println("LEFT short");
          }else if(rightButtonState == LOW){
            Serial.println("RIGHT short");
            activateShortBlinks(right);
          }
        }
        
			}
			buttonActive = false;
		}
	}

   unsigned long currentMillis = millis();

  if(isBlinking && currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;
    digitalWrite(ledBlinking, blinkState);
    blinkState = !blinkState;

    if(shortBlinking && blinks > 0){
      blinks--;
    } else if (shortBlinking && blinks==0){
      cancelTurnSignals();
    }
  }

  timer = millis();
  Vector norm = mpu.readNormalizeGyro();
  yaw = yaw + norm.ZAxis * timeStep;
  delay((timeStep*1000) - (millis() - timer));

  if(!shortBlinking && isBlinking){
    if(!isTurned && (yaw > TURN_QUALIFIER || yaw < -TURN_QUALIFIER)){
      isTurned = true;
    } else if (isTurned && ((ledBlinking == left && yaw < CANCELATION_ANGLE) || (ledBlinking == right && yaw > -CANCELATION_ANGLE))){
      cancelTurnSignals();
    }
  }
}