#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>

int lock = D1;
#define LED_R D0 
#define TRIG_PIN D2
#define ECHO_PIN D8
#define ACCESS_DELAY 2000
#define DENIED_DELAY 1000
#define RST_PIN D3   
#define SS_PIN D4
#define SSID "Galaxy A727885"
#define PASSWORD "ginnamon"
MFRC522 mfrc522(SS_PIN, RST_PIN); // Instance of the class

bool isSolenoidActive = false; // Track solenoid status
bool isFirstTap = true;        // Track if it's the first RFID tap

void setup() {
  Serial.begin(9600);
  pinMode(lock, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_R, OUTPUT);
  digitalWrite(lock, HIGH);
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

  if (distance_cm < 50) {
    digitalWrite(LED_R, HIGH);
    Serial.println("Ada");
  }
  if (distance_cm > 50) {
    digitalWrite(LED_R, LOW);
    Serial.println("Kosong");
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

  if (content.substring(1) == "B4 E6 81 07") {
    Serial.println("RFID tapped");

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
}
