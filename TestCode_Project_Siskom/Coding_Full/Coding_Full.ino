// Library
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <LiquidCrystal_I2C.h>
//#include "RTClib.h"
#include "ThingSpeak.h"
#include <BlynkSimpleEsp8266.h>


/*
   ANALOG
   Soil Moisture       A0

*/

/* DIGITAL
   DHT11               D3
   Ultrasonic HC-SR04  Trig D8 / Echo D7
*/

/*
   SDA                 D2
   SCL                 D1
   GND                 GND
   VCC                 5V
*/

/*
   Relay_1            D4
   Relay_2            D5
   Relay Fan          D6
   VCC                5V
   GND                GND
*/


// Soil Moisture
#define soilMoisture A0
int valueMoisture;

// Relay
#define relayPump_1 D4 
#define relayPump_2 D5
#define relayFan    D6
int relayStateON = LOW;
int relayStateOFF = HIGH;
bool Relay = 0;

//Enter your WIFI SSID
char ssid[] = "RedmiN11";
char pass[] = "Istiqomah";  //Enter your WIFI Password
char auth[] = "6qeAfw_7kNjfk3LwhHyWqq-AjparhSM4";//Enter your Auth token

WiFiClient client;

unsigned long myChannelNumber = 2508570; // channel ID
const char * myWriteAPIKey = "8EJYTFQX6G8YIXQV"; // write APIKey

// DHT11
#include "DHT.h"
#define DHTPIN D3
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);
int kelembaban = 0;
int suhu = 0;

// Ultrasonic HC-SR04
#define trigPin D8
#define echoPin D7

long duration;
int distance;

LiquidCrystal_I2C lcd(0x27, 20, 4);

// RTC DS3231
//RTC_DS3231 rtc;
//char dataHari[7][12] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"};
//String hari;
//int tanggal, bulan, tahun, jam, menit, detik;

//BlynkTimer timer;

void setup() {
  Serial.begin(9600);
  Wire.begin(D2, D1);
  dht.begin();
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  lcd.init();
  lcd.backlight();

  pinMode(relayPump_1, OUTPUT);
  pinMode(relayPump_2, OUTPUT);
  pinMode(relayFan, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  digitalWrite(relayPump_1, relayStateOFF);
  digitalWrite(relayPump_2, relayStateOFF);
  digitalWrite(relayFan, relayStateOFF);
  //  digitalWrite(relayPump, HIGH);

  // Initialize ThingSpeak
  ThingSpeak.begin(client);

  // inisialisasi RTC DS3231
//  if (! rtc.begin()) {
//    Serial.println("RTC Tidak Ditemukan");
//    Serial.flush();
//    abort();
//  }
//
//  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  WiFi.mode(WIFI_STA);

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
  lcd.print("Di : ");
  lcd.setCursor(11, 0);
  lcd.print("Mo : ");
  lcd.setCursor(0, 1);
  lcd.print("Te : ");
  lcd.setCursor(11, 1);
  lcd.print("Hu : ");
  lcd.setCursor(0, 2);
  lcd.print("P1 : OFF ");
  lcd.setCursor(11, 2);
  lcd.print("P2 : OFF ");
  lcd.setCursor(0, 3);
  lcd.print("Fan : OFF ");

  //Call the function
//  timer.setInterval(100L, valueMoisture);
}

// Get the button value
BLYNK_WRITE(V4){
  Relay = param.asInt();

  if(Relay == 1){
    digitalWrite(relayFan, relayStateON);
  } else {
    digitalWrite(relayFan, relayStateOFF);
  }
}

void loop() {
  moisture();
  temp();
  ultrasonic();
//  bacartc();

  Blynk.run();//Run the Blynk library
//  timer.run();//Run the Blynk timer

  // print nilai sensor
  lcd.setCursor(5, 0);
  lcd.print(distance);
  lcd.setCursor(8, 0);
  lcd.print("cm");
  lcd.setCursor(16, 0);
  lcd.print(valueMoisture);
  lcd.setCursor(19, 0);
  lcd.print("%");
  lcd.setCursor(5, 1);
  lcd.print(suhu);
  lcd.setCursor(8, 1);
  lcd.print ((char)223);
  lcd.print ("C");
  lcd.setCursor(16, 1);
  lcd.print(kelembaban);
  lcd.setCursor(19, 1);
  lcd.print("%");
//  lcd.setCursor(0, 3);
//  lcd.print((String() + tanggal + "/" + bulan + "/" + tahun));
//  lcd.setCursor(12, 3);
//  lcd.print((String() + jam + ":" + menit + ":" + detik));
  

  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  // set the fields with the values
  ThingSpeak.setField(1, suhu);
  ThingSpeak.setField(2, kelembaban);
  ThingSpeak.setField(3, valueMoisture);

//  if (jam == 9 && menit == 00 && detik == 00) {
//    digitalWrite(relayPump_1, relayStateON);
//    delay(10000);
//    digitalWrite(relayPump_1, relayStateOFF);
//  }
//
//  if (jam == 15 && menit == 00 && detik == 00) {
//    digitalWrite(relayPump_1, relayStateON);
//    delay(10000);
//    digitalWrite(relayPump_1, relayStateOFF);
//  }

  if (valueMoisture < 30 && valueMoisture <= 60) { // 70
    digitalWrite(relayPump_1, relayStateON);
    //    digitalWrite(relayPump, LOW);
    lcd.setCursor(0, 2);
    lcd.print("P1 : ON ");
    Serial.println("P1 : ON");
    //  } else if (valueMoisture <= 30) { // 70
    //    digitalWrite(relayPump_1, relayStateON);
    //    lcd.setCursor(0, 2);
    //    lcd.print("P1 : ON ");
    //    Serial.println("P1 : ON");
  } else {
    digitalWrite(relayPump_1, relayStateOFF);
    lcd.setCursor(0, 2);
    lcd.print("P1 : OFF ");
    Serial.println("P1 : OFF");
  }

  // The distance based on the container that used
  if (distance >= 7 && distance <= 9) {
    digitalWrite(relayPump_2, relayStateON);
    lcd.setCursor(11, 2);
    lcd.print("P2 : ON ");
    Serial.println("P2 : ON");
  } else if (distance == 3) {
    digitalWrite(relayPump_2, relayStateOFF);
    lcd.setCursor(11, 2);
    lcd.print("P2 : OFF ");
    Serial.println("P2 : OFF");
  } else {
    digitalWrite(relayPump_2, relayStateOFF);
    lcd.setCursor(11, 2);
    Serial.println("P2 : OFF ");
  }

  if (suhu >= 27) {
    digitalWrite(relayFan, relayStateON);
    lcd.setCursor(0, 3);
    lcd.print("Fan : ON ");
  } else if (suhu < 27) {
    digitalWrite(relayFan, relayStateOFF);
  } else {
    digitalWrite(relayFan, relayStateOFF);
    lcd.setCursor(0, 3);
    lcd.print("Fan : OFF ");
  }

  delay(1250);
}

void moisture() {
  valueMoisture = analogRead(soilMoisture);
  valueMoisture = map(valueMoisture, 0, 1024, 0, 100);
  valueMoisture = (valueMoisture - 100) * -1; // diubah dari nilai minus menjad positif

  Blynk.virtualWrite(V0, valueMoisture);

  Serial.print("Moisture : ");
  Serial.print(valueMoisture);
  Serial.println();
}

void temp() {
  kelembaban = dht.readHumidity();
  suhu = dht.readTemperature();

  Blynk.virtualWrite(V1, suhu);
  Blynk.virtualWrite(V2, kelembaban);

  Serial.print("Kelembaban : ");
  Serial.println(kelembaban);
  Serial.print("Suhu : ");
  Serial.println(suhu);
}

void ultrasonic() {
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // Calculating the distance, 340 m/s to cm/microsecond = 34/10^-3 = 0,034 cm
  distance = duration * 0.034 / 2;

  Blynk.virtualWrite(V3, distance);
  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
}

//void bacartc() {
//  DateTime now = rtc.now();
//  hari    = dataHari[now.dayOfTheWeek()];
//  tanggal = now.day(), DEC;
//  bulan   = now.month(), DEC;
//  tahun   = now.year(), DEC;
//  jam     = now.hour(), DEC;
//  menit   = now.minute(), DEC;
//  detik   = now.second(), DEC;
//
//  Serial.println(String() + hari + ", " + tanggal + "-" + bulan + "-" + tahun);
//  Serial.println(String() + jam + ":" + menit + ":" + detik);
//  delay(1000);
//}
