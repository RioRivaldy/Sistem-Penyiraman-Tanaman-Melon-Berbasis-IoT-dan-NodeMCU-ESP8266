int PIR = 4;
int statusPin = 0;

void setup()
{
  Serial.begin(9600);
  pinMode(PIR, INPUT);
}

void loop()
{
  statusPin = digitalRead(PIR);
  if (statusPin == HIGH)
  {
    Serial.println("Gerakan Terdeteksi");
  }
  else
  {
    Serial.println("Tidak ada Gerakan");
  }

  delay(1000);
}
