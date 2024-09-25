#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <EmonLib.h>                   // Include Emon Library

EnergyMonitor currentSensor;                   // Create an instance
Adafruit_BME280 bme;

// #define SEALEVELPRESSURE_HPA (1013.25)

unsigned long previousMillis1 = 0;
unsigned long previousMillis2 = 0;

extern double currCalibration = 11;  //0.52   111.1
extern float temp = 0;
extern float hum = 0;
extern double Irms = 0;
void updateSensorTemp_Hum(){
  unsigned long currentMillis1 = millis();
  if (currentMillis1 - previousMillis1 >= 1000) {
    previousMillis1 = currentMillis1;
    temp = bme.readTemperature();
    hum = bme.readHumidity();
    // Serial.print(temp);
    // Serial.print("        ");
    // Serial.print(hum);
    // Serial.println("        ");
  }
}
void updateSensorCurrent(){
  unsigned long currentMillis2 = millis();
  if (currentMillis2 - previousMillis2 >= 100) {
    previousMillis2 = currentMillis2;
    Irms = currentSensor.calcIrms(1480);  // Calculate Irms only
    // Serial.print(Irms*230.0);	       // Apparent power
    // Serial.print("W");
    // Serial.print(" ");
    Serial.print(Irms);		       // Irms
    Serial.print(" ");
    Serial.println("A");
  }
}