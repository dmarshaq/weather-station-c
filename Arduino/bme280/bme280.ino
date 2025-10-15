#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme;

void setup() {
  Serial.begin(9600);
  if(!bme.begin()){
    while(1);
  }

}

void loop() {
  float temp = bme.readTemperature(); //Degrees celsius
  float pressure = bme.readPressure(); //Pascals
  float humidity = bme.readHumidity(); //Percent
  Serial.println((String)temp + " " + (String)pressure + " " + (String)humidity);
  delay(1000);
}
