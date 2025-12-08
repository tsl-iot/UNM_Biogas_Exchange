/* 
 * Project myProject
 * Author: Your Name
 * Date: 
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"
#include "Adafruit_VEML7700.h"

// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(SEMI_AUTOMATIC);

Adafruit_VEML7700 luxSensor;

float getLux();
void initVEML7700();

float luxVal;

void setup() {
  Serial.begin(9600);
  waitFor(Serial.isConnected, 5000);
  initVEML7700();
}

void loop() {
  luxVal = getLux();
  Serial.printf("%0.2f\n", luxVal);
  delay(5000);
}

float getLux(){
  return luxSensor.readLux();
}

void initVEML7700(){
  if(!luxSensor.begin()){
    Serial.printf("VEML7700 not recognized\n");
  }
  else{
    Serial.printf("VEML up and running!\n");
  }
  luxSensor.setGain(VEML7700_GAIN_1_8);
  luxSensor.setIntegrationTime(VEML7700_IT_100MS);
}

