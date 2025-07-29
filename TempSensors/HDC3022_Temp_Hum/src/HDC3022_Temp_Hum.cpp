/* 
 * Project myProject
 * Author: Your Name
 * Date: 
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"
#include <Adafruit_HDC302x.h>

SYSTEM_MODE(SEMI_AUTOMATIC);

Adafruit_HDC302x hdc = Adafruit_HDC302x();

void setup() {
  Serial.begin(9600);

  Serial.printf("Adafruit HDC302x Simple Test");

  if (!hdc.begin(0x44)) {
    Serial.printf("Could not find sensor?");
    //while (1);
  }
  delay(1000);
}

void loop() {
  double temp = 0.0;
  double RH = 0.0;
  float tempF;

  hdc.readTemperatureHumidityOnDemand(temp, RH, TRIGGERMODE_LP0);
  tempF = (temp*9.0/5.0) + 32;
  Serial.printf("Temp: %0.1f\nHumidity: %0.1f\n", tempF, RH);
  delay(2000);
}

