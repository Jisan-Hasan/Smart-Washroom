#include <SoftwareSerial.h>


#define BLYNK_TEMPLATE_ID "TMPL6E_SG5gud"
#define BLYNK_TEMPLATE_NAME "Micro"
#define BLYNK_AUTH_TOKEN "LgU44FqI6-fMlZeK9d4472J8Wu5G3Hyp"


#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

SoftwareSerial mySerial(D5, D6);  // RX, TX

char auth[] = BLYNK_AUTH_TOKEN;

char ssid[] = "Connecting...";
char pass[] = "No@Password";


int value1, value2, washroomUsedCount;
float value3;

int handWashLevel = 0;



void setup() {
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);
  mySerial.begin(9600);
}

void loop() {
  if (mySerial.available()) {
    String message = mySerial.readString();

    // Use sscanf to extract the values from the message
    sscanf(message.c_str(), "%d,%d,%f,%d", &value1, &value2, &value3, &washroomUsedCount);

    // calculate handWashLevel
    if (value3 > 5) {
  handWashLevel = 0;
}
// Value3 is between 1 and 6 (inclusive)
else if (value3 >= 1 && value3 <= 5) {
  handWashLevel = 100 - ((value3 - 1) * 25); 
}
// Value3 is less than 1
else {
  handWashLevel = 100;
}

    Blynk.virtualWrite(V0, value1);
    Blynk.virtualWrite(V1, handWashLevel);
    Blynk.virtualWrite(V2, value2);
    Blynk.virtualWrite(V3, washroomUsedCount);
  }
}
