#include <ESP8266WiFi.h>

char ssid[] = "Kos Biru"; 
char pass[] = "Bagyo123";  //Enter your WIFI Password

// DHT11
#include "DHT.h"
#define DHTPIN D3
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);
int kelembaban, suhu;

void setup() {
  Serial.begin(115200);
  dht.begin();

  // Memulai konfigurasi WiFi
  WiFi.begin(ssid, pass);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

}

void loop() {
  temp();

  delay(2000);
}

void temp() {
  kelembaban = dht.readHumidity();
  suhu = dht.readTemperature();

  Serial.print("Kelembaban : ");
  Serial.println(kelembaban);
  Serial.print("Suhu : ");
  Serial.println(suhu);
}
