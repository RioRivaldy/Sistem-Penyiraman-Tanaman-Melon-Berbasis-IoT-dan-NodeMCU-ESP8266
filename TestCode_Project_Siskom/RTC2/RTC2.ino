#include <Wire.h>
#include "RTClib.h"
#include<Servo.h>

Servo mekanik1;
Servo mekanik2;

RTC_DS3231 rtc;

char dataHari[7][12] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"};
String hari;
int tanggal, bulan, tahun, jam, menit, detik;

void setup () {
  Serial.begin(9600);

  mekanik1.attach(4);
  mekanik1.write(0);
  mekanik2.attach(5);
  mekanik2.write(0);

  if (! rtc.begin()) {
    Serial.println("RTC Tidak Ditemukan");
    Serial.flush();
    abort();
  }

  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0);
}

void bacartc(){
  DateTime now = rtc.now();
  hari    = dataHari[now.dayOfTheWeek()];
  tanggal = now.day(), DEC;
  bulan   = now.month(), DEC;
  tahun   = now.year(), DEC;
  jam     = now.hour(), DEC;
  menit   = now.minute(), DEC;
  detik   = now.second(), DEC;
}

void servoputar(int jumlah){
  for (int i=1; i <= jumlah; i++){
    mekanik1.write(180);
    mekanik2.write(180);
    delay(200);
    mekanik1.write(0);
    mekanik2.write(0);
    delay(200);
}
}

void loop () {
  bacartc();
  
  Serial.println(String() + hari + ", " + tanggal + "-" + bulan + "-" + tahun);
  Serial.println(String() + jam + ":" + menit + ":" + detik);

  if (jam == 21 && menit == 19 && detik == 01){
    servoputar(10);
    }
  
  
  delay(1000);  
}



