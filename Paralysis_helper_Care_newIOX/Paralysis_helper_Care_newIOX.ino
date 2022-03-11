#include <WiFi.h>
#include <IOXhop_FirebaseESP32.h>
#include <Wire.h>
#include "MAX30105.h"

#include "heartRate.h"

MAX30105 particleSensor;

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred
int alerm = 2;   //  The on-board Arduion LED
float beatsPerMinute;
int beatAvg;
unsigned long lastTime = 0;
unsigned long timerDelay = 15000;
const String HE_NEEDS_YOU = "If the patient is in an emergency, he needs you!";
const String good_hel = "the patient is an good health, don't worry!";
const String importn_you = "He is want important problem meet him";
const String importn_you_f2 = "In case of an accident, pay attention to him immediately";

int Threshold = 120;

const int finger01 = 18;
const int finger02 = 5;
int state_f2; // 0 close - 1 open switch
int state_f1;

#define FIREBASE_HOST "https://prl-helper-system-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "urQKbEa3y37zA8s1ZjSuG5moNBZb3B9DhXtsT6cH"
#define WIFI_SSID "Dialog 4G 715"
#define WIFI_PASSWORD "4F2c67D3"

void setup() {
  Serial.begin(115200);
  pinMode(alerm, OUTPUT);
  pinMode(finger01, INPUT_PULLUP);
  pinMode(finger02, INPUT_PULLUP);
  Serial.println("Initializing...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  Wire.begin(15, 4);

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop() {

  long irValue = particleSensor.getIR();
  state_f1 = digitalRead(finger01);
  state_f2 = digitalRead(finger02);
  //  Serial.println(state_f2);
  if (checkForBeat(irValue) == true) {

    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }

  //  Serial.print("IR=");
  //  Serial.print(irValue);
  //  Serial.print(", BPM=");
  //  Serial.print(beatsPerMinute);
  //  Serial.print(", Avg BPM=");
  //  Serial.print(beatAvg);

  //  if (irValue < 50000) {
  //    Serial.print(" No finger?");
  //    Serial.println();
  /* } else*/

  if ((millis() - lastTime) > timerDelay) {
    Firebase.set("/sensor/pulse_value", String(beatAvg));
    if (beatAvg > Threshold) {
      digitalWrite(alerm, HIGH);
      Firebase.set("/sensor/alerm", HE_NEEDS_YOU);
    }
    else {
      digitalWrite(alerm, LOW);
      Firebase.set("/sensor/alerm", good_hel);
    }
    lastTime = millis();

  }

  else if (state_f2 == LOW) {
    Serial.println("I am true2! man");
    Firebase.set("/sensor/Finger_inputs", importn_you);
    digitalWrite(alerm, HIGH);
    delay(5000);
    digitalWrite(alerm, LOW);
    Firebase.set("/sensor/Finger_inputs", good_hel);
  }

  else if (state_f1 == HIGH) {
    Serial.println("I am true! man");
    Firebase.set("/sensor/Finger_inputs", importn_you); // what is his want
    digitalWrite(alerm, HIGH);
    delay(5000);
    digitalWrite(alerm, LOW);
    Firebase.set("/sensor/Finger_inputs", good_hel);
  }

  //  else{
  //    Firebase.set("/sensor/Finger_inputs", good_hel);
  //  }
  //  }else if (state_f1 == HIGH) {
  //    //    noTone(buzzer);
  //    digitalWrite(alerm, HIGH);
  //    Serial.println("I am true! man");
  //    Firebase.set("/sensor/Finger_inputs", importn_you); // what is his want
  //
  //
  //  }else if (state_f2 == HIGH) {
  //    //    tone(buzzer, 400);
  //    digitalWrite(alerm, LOW);
  //    Serial.println("I am False2! man");
  //    Firebase.set("/sensor/Finger_inputs_02", good_hel);
  //
  //  }
  //  else if (state_f2 == LOW) {
  //    //    noTone(buzzer);
  //    digitalWrite(alerm, HIGH);
  //    Serial.println("I am true2! man");
  //    Firebase.set("/sensor/Finger_inputs_02", importn_you_f2); // what is his want
  //
  //
  //  }
}
