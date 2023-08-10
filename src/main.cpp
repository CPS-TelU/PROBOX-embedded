#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
int lock = 27;
#define LED_G 5 //define green LED pin
#define LED_R 4 //define red LED
#define ACCESS_DELAY 2000
#define DENIED_DELAY 1000
#define RST_PIN D3   
#define SS_PIN D4
#define SSID "iPhone"
#define PASSWORD "yakaligaskuy"
MFRC522 mfrc522(SS_PIN, RST_PIN);// Instance of the class

void setup() {
  Serial.begin(9600);
  pinMode(lock, OUTPUT);
  digitalWrite(lock, HIGH);
  SPI.begin();		//protokol untuk mikrokontroler dengan memori	
	mfrc522.PCD_Init(); //inialisasi modulRFID
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("SUCCESS!!!");
}

void loop() {
if ( ! mfrc522.PICC_IsNewCardPresent()) { //memeriksa kartu
    return;
	}
	if ( ! mfrc522.PICC_ReadCardSerial()) { //membaca kartu
    return;
	}
  // Menampilkan UID di serial monitor
  Serial.print("UID tag :");
  String content= "";
  for (byte i = 0; i < mfrc522.uid.size; i++) //loop sebanyak byte yang ada pada UID
  {
  Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
  Serial.print(mfrc522.uid.uidByte[i], HEX);
  content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
  content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  content.toUpperCase();
  if (content.substring(1) == "B4 E6 81 07") //change here the UID of the card/cards that you want to give access
  {
    Serial.println("Authorized access");
    Serial.println();
    delay(500);
    digitalWrite(lock, LOW);
    digitalWrite(LED_G, HIGH);
    delay(ACCESS_DELAY);
    digitalWrite(lock, LOW);
    digitalWrite(LED_G, LOW);
  }
 
 else   {
    Serial.println(" Access denied");
    digitalWrite(lock, HIGH);
    digitalWrite(LED_R, HIGH);
    delay(DENIED_DELAY);
    digitalWrite(LED_R, LOW);
  }
  delay(1000);
  digitalWrite(lock, HIGH);
  
}