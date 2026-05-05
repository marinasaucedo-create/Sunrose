#include <Wire.h>
#include <Adafruit_BMP085.h>

Adafruit_BMP085 bmp;

void setup() {
  Serial.begin(115200);
  Wire.begin(4, 5); // SDA, SCL

  if (!bmp.begin()) {
    Serial.println("No se encontró el BMP180");
    while (1);
  }
}

void loop() {
  Serial.print("Temperatura: ");
  Serial.print(bmp.readTemperature());
  Serial.println(" °C");

  Serial.print("Presión: ");
  Serial.print(bmp.readPressure());
  Serial.println(" Pa");

  delay(2000);
}