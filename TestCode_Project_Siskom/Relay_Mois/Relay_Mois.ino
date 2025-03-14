// Library
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <LiquidCrystal_I2C.h>

#define soilMoisture A0
#define relayPump D3

//Enter your WIFI SSID
char ssid[] = "Kos Biru";
char pass[] = "Bagyo123";  //Enter your WIFI Password

int relayStateON = LOW;
int relayStateOFF = HIGH;
int valueMoisture;

LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
  Serial.begin(9600);
  Wire.begin(D2, D1);
  lcd.init();
  lcd.backlight();

  pinMode(relayPump, OUTPUT);
  digitalWrite(relayPump, relayStateOFF);
  //  digitalWrite(relayPump, HIGH);

  WiFi.begin(ssid, pass); // Ganti dengan nama dan kata sandi WiFi Anda
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  lcd.setCursor(0, 0);
  lcd.print("  Initializing  ");
  for (int a = 5; a <= 10; a++) {
    lcd.setCursor(a, 1);
    lcd.print(".");
    delay(500);
  }
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("SMART WATERING");
  lcd.setCursor(0, 1);
  lcd.print("Mois : ");
  lcd.setCursor(0, 2);
  lcd.print("Pump : OFF ");
}

void loop() {
  moisture();

  // print nilai sensor
  lcd.setCursor(8, 1);
  lcd.print(valueMoisture);

  if (valueMoisture > 10) {
    digitalWrite(relayPump, relayStateON);
    //    digitalWrite(relayPump, LOW);
    lcd.setCursor(0, 2);
    lcd.print("Pump : ON ");
    Serial.println("Pump : ON");
//  } else if (valueMoisture <= 70) {
//    digitalWrite(relayPump, relayStateON);
//    lcd.setCursor(0, 2);
//    lcd.print("Pump : ON ");
//    Serial.print("Pump : ON");
  } else {
    digitalWrite(relayPump, relayStateOFF);
    lcd.setCursor(0, 2);
    lcd.print("Pump : OFF ");
    Serial.println("Pump : OFF");
  }

  delay(1250);
}

void moisture() {
  valueMoisture = analogRead(soilMoisture);
  valueMoisture = map(valueMoisture, 0, 1024, 0, 100);
  valueMoisture = (valueMoisture - 100) * -1; // diubah dari nilai minus menjad positif

  Serial.print("Moisture : ");
  Serial.print(valueMoisture);
  Serial.println();
}
