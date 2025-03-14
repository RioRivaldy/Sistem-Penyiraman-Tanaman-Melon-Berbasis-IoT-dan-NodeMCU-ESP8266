#include <ESP8266WiFi.h>

#define trigPin D8
#define echoPin D7

long duration;
int distance;

// Relay
#define relayPump D4
int relayStateON = LOW;
int relayStateOFF = HIGH;

//Enter your WIFI SSID
char ssid[] = "Kos Biru";
char pass[] = "Bagyo123";  //Enter your WIFI Password

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(relayPump, OUTPUT);
  digitalWrite(relayPump, relayStateOFF);

  WiFi.begin(ssid, pass); // Ganti dengan nama dan kata sandi WiFi Anda
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
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
  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  
  // The distance based on the container that used
  if (distance >= 20 && distance <= 25) {
    digitalWrite(relayPump, relayStateON);
    Serial.println("Pompa Menyala");
  } else if (distance == 26) {
    digitalWrite(relayPump, relayStateOFF);
    Serial.println("Pompa Mati");
  }

  delay(2000);
}
