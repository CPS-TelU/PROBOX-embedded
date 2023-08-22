#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>

const int lock = D1;
const int buzzer = D9;
const int LED_R = D10;
const int button = D0; 
#define TRIG_PIN D2
#define ECHO_PIN D8
#define ACCESS_DELAY 2000
#define DENIED_DELAY 1000
#define RST_PIN D3   
#define SS_PIN D4
#define SSID "iPhone"
#define PASSWORD "yakaligaskuy"
MFRC522 mfrc522(SS_PIN, RST_PIN); // Instance of the class

//bool isSolenoidActive = false; // Track solenoid status
bool isFirstTap = true;        // Track if it's the first RFID tap
bool refresh = false;

void setup() {
  Serial.begin(9600);
  pinMode(lock, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(button, INPUT_PULLUP);
  digitalWrite(lock, HIGH);
  digitalWrite(LED_R, LOW);
  noTone(buzzer);
  isFirstTap = true;
  SPI.begin();
  mfrc522.PCD_Init();
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("SUCCESS!!!");
}

void loop() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  int distance_cm = (duration / 2) / 29.1;

  Serial.print("Distance: ");
  Serial.print(distance_cm);
  Serial.println(" cm");
  delay(1000);

  if (distance_cm < 30) {
    digitalWrite(LED_R, HIGH);
    Serial.println("Ada");
  }
  if (distance_cm > 30) {
    digitalWrite(LED_R, LOW);
    Serial.println("Kosong");
  }

  if (digitalRead(button) == LOW) {
    refresh = true;
  }
  else if (refresh) {
      Serial.println("System refreshed");
      digitalWrite(lock, LOW);
      Serial.print("Solenoid deactivated");
      delay(1000);
      ESP.restart();
      refresh = false;
    }

  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.print("UID tag :");
  String content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  content.toUpperCase();
  
  if (content.substring(1) == "B4 E6 81 07" || content.substring(1) == "13 1F FB 0B") {
    Serial.println("RFID tapped");
    Serial.println("Authorized access (orang1)");

    if (isFirstTap) {
      Serial.println("Solenoid activated");
      digitalWrite(lock, LOW);
      isFirstTap = false;
    } else {
      Serial.println("Solenoid deactivated");
      digitalWrite(lock, HIGH);
      isFirstTap = true;
    }

    delay(1000); // Delay to prevent rapid toggling due to reading multiple taps
  }
  else   {
    Serial.println("Access denied");
    digitalWrite(buzzer, HIGH);
    delay(1000);
    digitalWrite(buzzer, LOW);
  }
}

