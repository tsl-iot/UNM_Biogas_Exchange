/* 
 * Project myProject
 * Author: Your Name
 * Date: 
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"
#include <Adafruit_MLX90614.h>
// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(SEMI_AUTOMATIC);


Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void setup() {
  Serial.begin(9600);

  Serial.printf("Adafruit MLX90614 test");

  mlx.begin();
}

void loop() {
  // Serial.print("Ambient = "); Serial.print(mlx.readAmbientTempC());
  // Serial.print("*C\tObject = "); Serial.print(mlx.readObjectTempC()); Serial.println("*C");
  // Serial.print("Ambient = "); Serial.print(mlx.readAmbientTempF());
  // Serial.print("*F\tObject = "); Serial.print(mlx.readObjectTempF()); Serial.println("*F");

  Serial.printf("Ambient temp *F: %0.1f\nObject Temp *F: %0.1f\n", mlx.readAmbientTempF(), mlx.readObjectTempF());

  delay(500);
}
