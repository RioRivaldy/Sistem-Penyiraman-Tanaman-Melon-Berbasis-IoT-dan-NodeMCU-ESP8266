// Library
#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include "time.h"
#include <LiquidCrystal_I2C.h> // Library untuk LCD I2C
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert Firebase project API Key
#define API_KEY "AIzaSyAdFRGSJkmsR36UGlCZpDw_ZAxtA9xMscg"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "mariorivaldmariorivaldi2002@gmail.com"
#define USER_PASSWORD "mezutozil999"

// Insert RTDB URL
#define DATABASE_URL "https://datatraining-381808-default-rtdb.firebaseio.com/"

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variable to save USER UID
String uid;

// Database main path (to be updated in setup with the user UID)
String databasePath;
String database_1Path;

// Database child nodes
// DHT11
String celciusPath = "/celcius";
String humidityPath = "/humidity";
// Moisture
String moisturePath = "/moisture";
// Data BMKG
String weatherPath = "/weather";
String humidPath = "/humid";
String tempcPath = "/tempc";
String tempfPath = "/tempf";
String timeweatherPath = "/time";
String weathercodePath = "/weathercode";
// Timestamp
String timePath = "/timestamp";

// Parent Node (to be updated in every loop)
String parentPath;
String dataPath;

FirebaseJson json;

const char* ntpServer = "pool.ntp.org";

// Insert your network credentials
#define WIFI_SSID "Kos Biru"
#define WIFI_PASSWORD "Bagyo123"

// DHT11
#include "DHT.h"
#define DHTPIN D3
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);
int kelembaban, suhu;

//Relay
const int relayPump = D4;
int relayStateON = LOW;
int relayStateOFF = HIGH;

// Moisture
#define soilMoisture A0
int valueMoisture;

// Inisialisasi LCD
LiquidCrystal_I2C lcd(0x27, 20, 4); // Alamat I2C dan ukuran LCD (20x4)

// JSON Library
DynamicJsonDocument doc(1536);

// Inisialisasi variabel boolean untuk menentukan apakah program harus memeriksa kelembaban tanah
bool checkMoisture = true;

// Server API JSON untuk weather daerah Depok
const char *serverName = "https://ibnux.github.io/BMKG-importer/cuaca/5002229.json";
String weatherReading;

// Timer variables (send new readings every three minutes)
unsigned long lastTime = 0;
unsigned long timerDelay = 10000;
unsigned long intervalMois = 1000; // 1 detik
unsigned long previousTime = 0;
// Firebase
unsigned long sendDataPrevMillis = 0;

// Fungsi untuk menampilkan data pada LCD
void lcdDisplay(int row, int col, String text) {
  lcd.setCursor(col, row);
  lcd.print(text);
}

// Fungsi untuk melakukan request ke API JSON
String httpGETRequest(const char *serverName) {
  WiFiClientSecure httpsClient;
  HTTPClient http;

  httpsClient.setInsecure();
  httpsClient.connect(serverName, 443);

  http.begin(httpsClient, serverName);

  String payload;
  int response = http.GET();
  if (response == HTTP_CODE_OK) {
    payload = http.getString();
  } else {
    Serial.print("Error code: ");
    Serial.println(response);
  }

  http.end();
  return payload;
}

// Function that gets current epoch time and formats it to local time
String getFormattedTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return String("Failed to obtain time");
  }
  char timeStringBuff[50];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%d/%m/%Y %H:%M:%S", &timeinfo);
  return String(timeStringBuff);
}

void setup() {
  Serial.begin(9600);
  pinMode(relayPump, OUTPUT);
  digitalWrite(relayPump, relayStateOFF);

  configTime(7 * 3600, 0, ntpServer); // GMT+7
  dht.begin();

  // Memulai konfigurasi WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  // Inisialisasi LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  delay(2000);

  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path
  databasePath = "/UsersData/" + uid;

  database_1Path = "/Data Sensor & Forecasts Weather/";
}

void loop() {
  dht11();
  moisture();

  if ((millis() - lastTime) > timerDelay) {
    lcd.clear();
    // Periksa status WiFi
    if (WiFi.status() == WL_CONNECTED) {
      // Menampilkan Text "Fetching..." ke LCD
      lcdDisplay(0, 0, "Fetching..");

      // Melakukan Request ke API JSON
      weatherReading = httpGETRequest(serverName);
      Serial.println("---- REQUEST RESULT FROM API ----");
      Serial.println(weatherReading);

      // Proses deserialize
      deserializeJson(doc, weatherReading);

      lcd.clear();

      // Iterasi untuk menunjukan semua elemen yang ada di JSON
      for (JsonObject item : doc.as<JsonArray>()) {
        const char* jamCuaca = item["jamCuaca"]; // "2024-06-01 00:00:00", "2024-06-01 06:00:00", "2024-06-01 ...
        const char* kodeCuaca = item["kodeCuaca"]; // "3", "60", "3", "1", "1", "61", "1", "1", "1", "60", "3", ...
        const char* cuaca = item["cuaca"]; // "Berawan", "Hujan Ringan", "Berawan", "Cerah Berawan", "Cerah ...
        const char* humidity = item["humidity"]; // "95", "75", "95", "95", "80", "55", "80", "90", "80", "55", ...
        const char* tempC = item["tempC"]; // "25", "30", "25", "25", "25", "33", "25", "23", "25", "32", "25", ...
        const char* tempF = item["tempF"]; // "77", "86", "77", "77", "77", "91", "77", "73", "77", "90", "77", ...


        lcdDisplay(0, 0, cuaca);
        lcdDisplay(1, 0, "Humidity: " + String(elem["humidity"]) + "%");
        lcdDisplay(2, 0, "Temp C  : " + String(elem["tempC"]) + "C");
        lcdDisplay(3, 0, String(elem["jamCuaca"]));

        delay(1500);
        lcd.clear();

        //        dataPath = database_1Path + "/";

        //        // Data DHT11
        //        Firebase.RTDB.setString(&fbdo, "Data Sensor & Forecasts Weather/suhu", String(suhu) + " °C");
        //        Firebase.RTDB.setString(&fbdo, "Data Sensor &  Forecasts Weather/kelembaban", String(kelembaban) + " %");
        //
        //        // Soil Moisture
        //        Firebase.RTDB.setString(&fbdo, "Data Sensor & Forecasts Weather/soil", String(valueMoisture) + + " %");
        //
        //        // Data BMKG
        //        Firebase.RTDB.setString(&fbdo, "Data Sensor & Forecasts Weather/humid", String(elem["cuaca"]));
        //        Firebase.RTDB.setString(&fbdo, "Data Sensor & Forecasts Weather/humid", String(elem["humidity"]) + " %");
        //        Firebase.RTDB.setString(&fbdo, "Data Sensor & Forecasts Weather/tempc", String(elem["tempC"]) + " °C");
        //        Firebase.RTDB.setString(&fbdo, "Data Sensor & Forecasts Weather/tempf", String(elem["tempF"]) + " °F");
        //        Firebase.RTDB.setString(&fbdo, "Data Sensor & Forecasts Weather/timeweather", String(elem["jamCuaca"]));
        //        Firebase.RTDB.setString(&fbdo, "Data Sensor & Forecasts Weather/weathercode", String(elem["kodeCuaca"]));


        dataPath = database_1Path + "/" + String(millis());  // Use millis() as unique identifier for each entry

        json.set(celciusPath.c_str(), String(suhu) + " °C");
        json.set(humidityPath.c_str(), String(kelembaban) + " %");
        json.set(moisturePath.c_str(), String(valueMoisture));
        json.set(weatherPath.c_str(), String(elem["cuaca"]));
        json.set(humidPath.c_str(), String(elem["humidity"] + " %"));
        json.set(tempcPath.c_str(), String(elem["tempC"] + " °C"));
        json.set(tempfPath.c_str(), String(elem["tempF"]));
        json.set(timeweatherPath.c_str(), String(elem["jamCuaca"]));
        json.set(weathercodePath.c_str(), String(elem["kodeCuaca"]));

        Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, dataPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());

        //logika mengatur pompa berdasarkan prediksi cuaca atau kode cuaca dari BMKG
        if (String(elem["kodeCuaca"] == "60" || elem["kodeCuaca"] == "61" || elem["kodeCuaca"] == "63" || elem["kodeCuaca"] == "80" || elem["kodeCuaca"] == "95" || elem["kodeCuaca"] == "97")) {
          digitalWrite(relayPump, relayStateOFF);
          //          Serial.println("Hujan mulai turun");
          // Set variabel checkMoisture menjadi false ketika kondisi cuaca terpenuhi
          checkMoisture = false;
        } else {
          // Set variabel checkMoisture menjadi true ketika kondisi cuaca tidak terpenuhi
          checkMoisture = true;
        }
      }
    } else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }

  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    //Get current formatted time
    String formattedTime = getFormattedTime();
    Serial.print("Formatted time: ");
    Serial.println(formattedTime);

    parentPath = databasePath + "/" + String(millis());  // Use millis() as unique identifier for each entry

    json.set(celciusPath.c_str(), String(suhu) + " °C");
    json.set(humidityPath.c_str(), String(kelembaban) + " %");
    json.set(timePath.c_str(), formattedTime);
    json.set(moisturePath.c_str(), String(valueMoisture));

    //    json.set(weatherPath.c_str(), String(elem["cuaca"]));
    //    json.set(humidPath.c_str(), String(elem["humidity"] + " %"));
    //    json.set(tempcPath.c_str(), String(elem["tempC"] + " °C"));
    //    json.set(tempfPath.c_str(), String(elem["tempF"]));
    //    json.set(timeweatherPath.c_str(), String(elem["jamCuaca"]));
    //    json.set(weathercodePath.c_str(), String(elem["kodeCuaca"]));

    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
  }

  // periksa kelembaban tanah ketika variabel checkMoisture bernilai true
  if (checkMoisture) {
    unsigned long currentTime = millis();
    if (currentTime - previousTime > intervalMois) {
      moisture();

      // periksa apakah tanah kering
      if (valueMoisture < 30) {
        digitalWrite(relayPump, relayStateON);
      } else if (valueMoisture <= 70) {
        digitalWrite(relayPump, relayStateON);
      } else {
        digitalWrite(relayPump, relayStateOFF);
      }
      previousTime = currentTime;
    }
  }
  delay(timerDelay);
}

void dht11() {
  kelembaban = dht.readHumidity();
  suhu = dht.readTemperature();

  Serial.print("Kelembaban : ");
  Serial.println(kelembaban);
  Serial.print("Suhu : ");
  Serial.println(suhu);
}

void moisture() {
  valueMoisture = analogRead(soilMoisture);
  valueMoisture = map(valueMoisture, 0, 1024, 0, 100);
  valueMoisture = (valueMoisture - 100) * -1; // diubah dari nilai minus menjad positif

  Serial.print("Moisture : ");
  Serial.print(valueMoisture);
  Serial.println();
}
