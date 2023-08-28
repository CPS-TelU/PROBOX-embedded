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
bool isFirstTap = true;        // Track if it's the first RFID tap
bool refresh = false;
const char* tap = "KUNCI"; // Instance of the class

// Replace with your network credentials
const char* ssid     = "hoka";
const char* password = "1408Hoka";

// supabase credentials
String API_URL = "https://pbzaiztldlpympnwfpuz.supabase.co";
String API_KEY = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InBiemFpenRsZGxweW1wbndmcHV6Iiwicm9sZSI6ImFub24iLCJpYXQiOjE2OTI3NjA1MTMsImV4cCI6MjAwODMzNjUxM30.tHDSYLyJkULk52TB8hM9rc6xYCse2xfu39RlgQjCIK0";
String TableName = "sensor";
const int httpsPort = 443;

// Sending interval of the packets in seconds
// int sendinginterval = 1200; // 20 minutes
// //int sendinginterval = 120; // 2 minutes

HTTPClient https;
WiFiClientSecure client;
void setup() {
  // builtIn led is used to indicate when a message is being sent
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // the builtin LED is wired backwards HIGH turns it off
  // HTTPS is used without checking credentials 
  client.setInsecure();
  // Connect to the WIFI 
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
  
  // Print local IP address
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
String uidString = "";
String status = "";
void loop() {

   // If connected to the internet turn the Builtin led On and attempt to send a message to the database 
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, LOW); // LOW turns ON

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
    // sendBoxData(); 
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
    if (uidString == "551E9552" || uidString == "584E0C53" || uidString == "BAE52B3B" || uidString == "1637C942" || uidString == "8518D952" ||
        uidString == "351C6452" || uidString == "782D9053" || uidString == "458B2552" || uidString == "48EFC453") {
      authorized = true;

      if (uidString == "551E9552") uidString = "AWP 21";
      else if (uidString == "584E0C53") uidString = "HZN 21";
      else if (uidString == "351C6452") uidString = "MFA 21";
      else if (uidString == "782D9053") uidString = "GHN 21";
      else if (uidString == "48EFC453") uidString = "ASF 21";
      else if (uidString == "458B2552") uidString = "OKA 21";
      else if (uidString == "BAE52B3B") uidString = "HRI 21";
      else if (uidString == "1637C942") uidString = "RIA 21";
      else if (uidString == "8518D952") uidString = "RHN 21";
    }
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
if (authorized) {
  // Send the post request to SUPABASE only for authorized RFID tags
  https.begin(client, API_URL + "/rest/v1/" + TableName);
  https.addHeader("Content-Type", "application/json");
  https.addHeader("Prefer", "return=representation");
  https.addHeader("apikey", API_KEY);
  https.addHeader("Authorization", "Bearer " + API_KEY);
  int httpCode = https.POST("{\"uid\":\"" + uidString + "\",\"status\":\"" + status + "\",\"selenoid\":\"" + tap + "\"}");
  String payload = https.getString(); 
  Serial.println(httpCode);   // Print HTTP return code
  Serial.println(payload);    // Print request response payload
  https.end();

  digitalWrite(LED_BUILTIN, HIGH); // HIGH turns off
} else {
  Serial.println("Access denied");
  digitalWrite(buzzer, HIGH); // Turn on the buzzer
  delay(1000);
  digitalWrite(buzzer, LOW); // Turn off the buzzer
}

// Delay before sending the next request
delay(1000);
  }
}
