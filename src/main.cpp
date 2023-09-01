#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>

const int lock = D1;
const int buzzer = 9;
const int LED_R = 10;
const int button = D0; 
#define TRIG_PIN D2
#define ECHO_PIN D8
#define ACCESS_DELAY 2000
#define DENIED_DELAY 1000
#define RST_PIN D3 
#define SS_PIN D4
MFRC522 mfrc522(SS_PIN, RST_PIN);

int val = 0;
int duration = 0;
int distance = 0;

const char* gktep = "";
bool isFirstTap = true;        
bool refresh = false;
const char* tap = "KUNCI"; 

const char* ssid     = "hoka";
const char* password = "1408Hoka";

String API_URL = ""; //api url
String API_KEY = ""; //apikey
String TableName = ""; //table name
const int httpsPort = 443;

HTTPClient https;
WiFiClientSecure client;
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); 
  client.setInsecure();
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
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
String uidString = "";
String status = "";
void loop() {

  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, LOW); 

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
    status = "ADA BARANG";
  }
  if (distance_cm > 30) {
    digitalWrite(LED_R, LOW);
    Serial.println("Kosong");
    status = "TIDAK ADA BARANG";
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
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  content.toUpperCase();

  uidString = content;

  
  bool authorized = false;
    if (uidString == "123" || uidString == "123" || uidString == "123" || uidString == "123" || uidString == "123" || uidString == "123" || uidString == "123" ||
        uidString == "123" || uidString == "123" || uidString == "123" || uidString == "123" || uidString == "123") { // change data from card registered
      authorized = true;

      if (uidString == "123") uidString = "ABC"; // change data
      else if (uidString == "123") uidString = "ABC";
      else if (uidString == "123") uidString = "ABC";
      else if (uidString == "123") uidString = "ABC";
      else if (uidString == "123") uidString = "ABC";
      else if (uidString == "123") uidString = "ABC";
      else if (uidString == "123") uidString = "ABC";
      else if (uidString == "123") uidString = "ABC";
      else if (uidString == "123") uidString = "ABC";
      else if (uidString == "123") uidString = "ABC";
      else if (uidString == "123") uidString = "ABC";
      else if (uidString == "123") uidString = "ABC";
    }
    
  
if (authorized) {

  if (isFirstTap) {
    Serial.println("Solenoid activated");
    digitalWrite(lock, LOW);
    isFirstTap = false;
  } else {
    Serial.println("Solenoid deactivated");
    digitalWrite(lock, HIGH);
    isFirstTap = true;
  }
  tap = isFirstTap ? "KUNCI" : "BUKA";

  https.begin(client, API_URL + "/rest/v1/" + TableName);
  https.addHeader("Content-Type", "application/json");
  https.addHeader("Prefer", "return=representation");
  https.addHeader("apikey", API_KEY);
  https.addHeader("Authorization", "Bearer " + API_KEY);
  int httpCode = https.POST("{\"uid\":\"" + uidString + "\",\"status\":\"" + status + "\",\"selenoid\":\"" + tap + "\"}");
  String payload = https.getString(); 
  Serial.println(httpCode);   
  Serial.println(payload);    
  https.end();

  digitalWrite(LED_BUILTIN, HIGH); 
} else {
  Serial.println("Access denied");
  digitalWrite(buzzer, HIGH); 
  delay(1000);
  digitalWrite(buzzer, LOW); 
}

delay(1000);
  }
}
