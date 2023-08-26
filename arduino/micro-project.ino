#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

SoftwareSerial nodemcu(2, 3);

LiquidCrystal_I2C lcd(0x3F, 16, 2);
Servo flushServo;
Servo liquidServo;

#define MQ2pin A0

const int washroomTrigPin = 8;
const int washroomEchoPin = 5;
const int washroomLightPin = A2;
const int motionSensorPin = 7;
const int commonLightPin = A3;
const int buzzerPin = 9;

const int liquidEchoPin = 10;
const int liquidTrigPin = 11;

const int levelEchoPin = 6;
const int levelTrigPin = 13;


int flushValue = 0;
int handWashValue = 0;
bool isSameHand = false;
bool handWashDropped = false;
float sensorValue;
int washroomUsedCount = 0;
int handWashLevel = 1;


int data1 = 0;
int data2 = 0;
float data3 = 0.0;

String cdata;

unsigned long lastDataSendTime = 0;           // Variable to track the last data send time
const unsigned long dataSendInterval = 1000;  // Data send interval in milliseconds


//define sound velocity in cm/uS
#define SOUND_VELOCITY 0.034
#define CM_TO_INCH 0.393701


void makeFlush() {
  if (flushValue) {
    washroomUsedCount++;
    flushServo.write(90);
    flushValue = 0;
    delay(3000);
    flushServo.write(0);
  }
}


float findDistance(int trigPin, int echoPin) {
  long duration;
  float distanceCm;
  float distanceInch;

  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // Calculate the distance
  distanceCm = duration * SOUND_VELOCITY / 2;

  // Convert to inches
  distanceInch = distanceCm * CM_TO_INCH;

  return distanceInch;
}

int checkUserInside() {
  float washroomUserDistance = findDistance(washroomTrigPin, washroomEchoPin);
  if (washroomUserDistance < 4.00) {
    data1 = 1;
    //Serial.println("Someone using this washroom!");
    flushValue = 1;
    // turn on washroom light & fan
    analogWrite(washroomLightPin, 255);

    return 1;
  } else {
    //Serial.println("Washroom is available to use!");
    data1 = 0;
    makeFlush();
    // turn off washroom light & fan
    analogWrite(washroomLightPin, 0);

    return 0;
  }
}




void setup() {
  pinMode(buzzerPin, OUTPUT);  //Set buzzerPin as output

  lcd.init();
  lcd.clear();
  lcd.backlight();

  nodemcu.begin(9600);

  Serial.begin(9600);  // Starts the serial communication
  flushServo.attach(4);
  liquidServo.attach(12);
  pinMode(washroomTrigPin, OUTPUT);
  pinMode(washroomEchoPin, INPUT);
  pinMode(washroomLightPin, OUTPUT);
  pinMode(motionSensorPin, INPUT);
  pinMode(commonLightPin, OUTPUT);

  pinMode(liquidEchoPin, INPUT);
  pinMode(liquidTrigPin, OUTPUT);

  pinMode(levelTrigPin, OUTPUT);
  pinMode(levelEchoPin, INPUT);

  flushServo.write(0);
  liquidServo.write(0);


  delay(1000);  //Add a little delay
}

void loop() {

  // check for smoke
  sensorValue = analogRead(MQ2pin);

  //Serial.print("total used -> ");
  //Serial.println(washroomUsedCount);

  //Serial.println(sensorValue);
  if (sensorValue > 300) {
    data2 = 1;
    beep(5);
  } else {
    data2 = 0;
    // digitalWrite(buzzerPin,HIGH);
  }

  //check koridor motion
  int val = digitalRead(motionSensorPin);
  if (val == 1) {
    digitalWrite(commonLightPin, HIGH);
  } else {
    digitalWrite(commonLightPin, LOW);
  }


  // check handWash liquid level
  float liquidLevel = findDistance(levelTrigPin, levelEchoPin);
  data3 = liquidLevel;
  if (liquidLevel < 4.00) {
    //Serial.println("Enough Handwash");
    handWashLevel = 1;
  } else {
    //Serial.println("Low Handwash. Please Refill!");
    handWashLevel = 0;
  }

  //
  handWashControl();
  int isUser = checkUserInside();
  // -----------------------------
  unsigned long currentTime = millis();

  // Send data at regular intervals
  if (currentTime - lastDataSendTime >= dataSendInterval) {
    lastDataSendTime = currentTime;

    // Collect data and create data string
    // ...
    cdata = cdata + data1 + "," + data2 + "," + data3 + "," + washroomUsedCount;

    // Send data to NodeMCU
    nodemcu.println(cdata);
    Serial.println(cdata);  // For debugging

    // Clear the data string
    cdata = String();
  }


  // ---------------------------------

  if (isUser) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("W-1 is Using");
    lcd.setCursor(0,1);
    lcd.print(washroomUsedCount);
    lcd.print(" times used");
  } else {
    lcd.setCursor(0,0);
    lcd.clear();
    lcd.print("W-1 is Free");
    lcd.setCursor(0,1);
    lcd.print(washroomUsedCount);
    lcd.print(" times used");
  }
  delay(500);
}

void dropHandWash() {
  if (handWashDropped == false) {
    liquidServo.write(90);
    delay(1000);
    liquidServo.write(0);
    handWashDropped = true;
  }
}

void handWashControl() {
  float userHandDistance = findDistance(liquidTrigPin, liquidEchoPin);
  if (userHandDistance < 4.00 && isSameHand == false) {
    //Serial.println("Handwash dropping");
    dropHandWash();
  } else {
    //Serial.println("Handwash Not dropping");
    handWashDropped = false;
  }
  // dropHandWash();
}


void beep(int x) {  //creating function
  unsigned char i, j;
  while (x) {
    for (i = 0; i < 80; i++)  // When a frequency sound
    {
      digitalWrite(buzzerPin, HIGH);  //send tone
      delay(1);
      digitalWrite(buzzerPin, LOW);  //no tone
      delay(1);
    }
    for (i = 0; i < 100; i++) {
      digitalWrite(buzzerPin, HIGH);
      delay(2);
      digitalWrite(buzzerPin, LOW);
      delay(2);
    }
    x--;
  }
}
