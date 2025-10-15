//Controls BME280 and LTR390

#include "Adafruit_LTR390.h"
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>

Adafruit_BME280 bme;
Adafruit_ltr390 ltr = Adafruit_LTR390();
const float UVScale = 1; //Scale to multiply raw UV index by to 

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
  Serial.print(ltr.readUVS());
  Serial.print(" ");
  //BME280 Temp
  Serial.print(bme.readTemperature());
  Serial.print(" ");
  //Pressure (Pascals)
  Serial.print(bme.readPressure());
  Serial.print(" ");
  Serial.print(bme.readHumidity());
  Serial.print("\n");
  delay(1000);
}
