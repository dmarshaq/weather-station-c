#include "Adafruit_LTR390.h"
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>

Adafruit_BME280 bme;
Adafruit_ltr390 ltr = Adafruit_LTR390();
const float UVScale = 1; //Scale to multiply raw UV index by to 
char out[16];

void setup() {
  Serial.begin(9600);
  //Set to UV mode instead of ambient light mode
  ltr.setMode(LTR390_MODE_UVS);
  //Sensitivity - Options are 1, 3, 6, 9, 18
  ltr.setGain(LTR390_GAIN_3);

  ltr.setResolution(LTR390_RESOLUTION_16BIT);
}

void loop() {
  //UV
  memset(out, 0x00, sizeof(out));
  float num = ltr.readUVS();
  out[0] = 0x07;
  memcpy(&out[1], &num, sizeof(float));
  Serial.write(out, 16);
  //BME280 Temp
  memset(out, 0x00, sizeof(out));
  num = bme.readTemperature();
  out[0] = 0x01;
  memcpy(&out[1], &num, sizeof(float));
  Serial.write(out, 16);
  //Pressure (Pascals)
  memset(out, 0x00, sizeof(out));
  num = bme.readPressure();
  out[0] = 0x05;
  memcpy(&out[1], &num, sizeof(float));
  Serial.write(out, 16);

  //Humidity
  memset(out, 0x00, sizeof(out));
  num = bme.readHumidity();
  out[0] = 0x02;
  memcpy(&out[1], &num, sizeof(float));
  Serial.write(out, 16);
  
  delay(1000);
}
