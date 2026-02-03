/* 
 * Project Main code that integrates temp/hum, and light sensors. Controls air flow by actuating solenoids. Used by UNM Bio Dept. 
 * Author: Edward Ishman, Jon Phillips, Adrian Pijoan, James Black --- Central New Mexico Community College's Technology Solutions Lab
 * Date: June 26th 2025
 * 
 *
 * 
 */

#include "Particle.h"
#include <Adafruit_GFX.h>
#include <Adafruit_HX8357.h>
#include "Adafruit_TSC2007.h"
#include <Adafruit_HDC302x.h>


#include "IoTClassroom_CNM.h"
#include "Adafruit_VEML7700.h"

#include "../lib/Adafruit_GFX_RK/src/Adafruit_GFX_RK.h"
#include "../lib/Adafruit_HX8357_RK/src/Adafruit_HX8357_RK.h"
#include "../lib/Adafruit_TSC2007/src/Adafruit_TSC2007.h"
#include "../lib/Adafruit_BusIO_Register/src/Adafruit_BusIO_Register.h"
#include "../lib/Adafruit_HDC302x/src/Adafruit_HDC302x.h"
#include "../lib/Adafruit_VEML7700/src/Adafruit_VEML7700.h"


SYSTEM_MODE(SEMI_AUTOMATIC);

// Constants
const int TFT_DC = D5;
const int TFT_CS = D4;
const int STMPE_CS = D3;
const int TFT_RST = -1;
const int TSC_IRQ = STMPE_CS;

const int TS_MINX = 100;
const int TS_MAXX = 3800;
const int TS_MINY = 100;
const int TS_MAXY = 3750;

const int DISPLAY_W = 480;
const int DISPLAY_H = 320;


const int SOLENOID_1PIN = D6;
const int SOLENOID_2PIN = D10;
const int SOLENOID_3PIN = D19;

const int LICORINPUTPIN = A5;
const int TC_PIN = A2;

// Variables
int16_t min_x, max_x, min_y, max_y;

static unsigned int lastPrint;

float baseTempReading;
double baseRHReading;
float chamberTempReading;
double chamberRHReading;
float leafThermoTemp;

int dataOriginCo2 = 76;
int dataOriginLux = 211;
int dataOriginLeafTemp = 345;
int dataOriginBaseTH = 76;
int dataOriginLeafTH = 278;
float luxReading;
float co2Val;




// Function Declarations
void hdc302xInit(int address_1, int address_2);
void displayInit();
void initVEML7700();
void initSolenoidValves(const int S1_PIN, const int S2_PIN, const int S3_PIN);
void layoutHomeScreen();
void readTS();
float getThermoTemp();
float getCO2();
float intoVolts(int bits);
float getLux();
void get_HDC_T_H(float *base_T, double *base_RH, float *chamber_T, double *chamber_RH);
void display_T_H(float bTemperature, float cTemperature, double bHum, double cHum);
void displayLeafData(float co2, float lux, float leaftTemp);

// Class Objects
Adafruit_HDC302x base_T_H = Adafruit_HDC302x();
Adafruit_HDC302x chamber_T_H = Adafruit_HDC302x();
Adafruit_HX8357 tft(TFT_CS, TFT_DC, TFT_RST); // for board 0 add the following: HX8357D, &SPI
Adafruit_TSC2007 ts; // newer rev 2 touch contoller
TS_Point p;
Adafruit_VEML7700_ luxSensor;

// Start of the program
void setup() {
  Serial.begin(9600);
  waitFor(Serial.isConnected, 5000);
  hdc302xInit(0x44, 0x47);
  displayInit();
  initVEML7700();
  layoutHomeScreen();
  initSolenoidValves(SOLENOID_1PIN, SOLENOID_2PIN, SOLENOID_3PIN);
  pinMode(LICORINPUTPIN, INPUT);
  pinMode(TC_PIN, INPUT);
  delay(2000);
}

void loop() {

  readTS();
  if((millis() - lastPrint) > 1000){
    co2Val = getCO2();
    luxReading = getLux();
    leafThermoTemp = getThermoTemp();
    displayLeafData(co2Val, luxReading, leafThermoTemp);
    display_T_H(baseTempReading, chamberTempReading, baseRHReading, chamberRHReading);
    Serial.printf("Base Temp: %0.1f\nBase RH: %0.1f\nChamber Temp: %0.1f\nChamber RH: %0.1f\nleaf temp: %0.1f\n", baseTempReading, baseRHReading, chamberTempReading, chamberRHReading, leafThermoTemp);
    lastPrint = millis();
  }
}

// Use address 0x44 for address_1 and 0x47 for address_2
void hdc302xInit(int baseAddress, int chamberAddress){
  if(!base_T_H.begin(baseAddress)){
    //Serial.printf("Could not find Base temp/hum sensor?");
    //while (1);
  }
  if(!chamber_T_H.begin(chamberAddress)){
    //Serial.printf("Could not find chamber temp/hum sensor?");
    //while (1);
  }
}

// Use address 0x48 for screenAddress
void displayInit(){
  if (!ts.begin(0x48)) {
    //Serial.printf("Couldn't start TSC2007 touchscreen controller\n");
  }
  else{
    //Serial.printf("Touchscreen started\n");
  }

  tft.begin();
  tft.setRotation(1);
  min_x = TS_MINX; max_x = TS_MAXX;
  min_y = TS_MINY; max_y = TS_MAXY;
  pinMode(TSC_IRQ, INPUT);
  
}



// Sets pinModes and initializes each solenoid to the LOW mode
void initSolenoidValves(const int S1_PIN, const int S2_PIN, const int S3_PIN){
  pinMode(S1_PIN, OUTPUT);
  pinMode(S2_PIN, OUTPUT);
  pinMode(S3_PIN, OUTPUT);

  digitalWrite(S1_PIN, LOW);
  digitalWrite(S2_PIN, LOW);
  digitalWrite(S3_PIN, LOW);
}

void initVEML7700(){
  if(!luxSensor.begin()){
    //Serial.printf("VEML7700 not recognized\n");
  }
  else{
    //Serial.printf("VEML up and running!\n");
  }
  luxSensor.setGain(VEML7700_GAIN_1_8);
  luxSensor.setIntegrationTime(VEML7700_IT_100MS);
}

// Draws the shapes that outlines the home screen
void layoutHomeScreen(){
  int menuRect_W = DISPLAY_W * .16;
  int barGraphBox_W = DISPLAY_W - menuRect_W;
  // int temp_hum_box_W = DISPLAY_W - menuRect_W;
  int barGraphBox_H = DISPLAY_H/2;
  // int temp_hum_box_H = DISPLAY_H/2;
  
  tft.setTextSize(3);
  tft.setRotation(1); // Landscape
  tft.fillScreen(HX8357_BLACK);

  tft.fillRect(0,0,menuRect_W,DISPLAY_H/2, HX8357_GREEN); // left panel option menu
  tft.fillRect(0,DISPLAY_H/2,menuRect_W,DISPLAY_H/2, HX8357_RED); // left panel option menu

  tft.drawRect(dataOriginCo2,0,(barGraphBox_W/3)+1,barGraphBox_H, HX8357_WHITE);
  tft.drawRect(dataOriginLux,0,(barGraphBox_W/3)+2,barGraphBox_H, HX8357_WHITE);  
  tft.drawRect(dataOriginLeafTemp, 0, (barGraphBox_W/3)+1, barGraphBox_H,HX8357_WHITE);
  tft.drawRect(76,160,(DISPLAY_W-76)/2,160, HX8357_WHITE);
  tft.drawRect(278,160,(DISPLAY_W-76)/2,160, HX8357_WHITE);
  tft.drawLine(76, 240, 480, 240, HX8357_WHITE);

  tft.setCursor(32, 74);
  tft.printf("C");
  tft.setCursor(32, 234);
  tft.printf("B");

  tft.setCursor(120, 12); 
  tft.printf("CO2");
  tft.setTextSize(2);
  tft.setCursor(130, 38); 
  tft.printf("ppm");
  tft.setTextSize(3);
  tft.setCursor(250, 12);
  tft.printf("Lux");
  tft.setTextSize(2); 
  tft.setCursor(355, 12);
  tft.printf("Leaf TempC");

  tft.setTextSize(2);
  tft.setCursor(117, 162);
  tft.printf("Base TempC\r");
  tft.setCursor(117, 242);
  tft.printf("Base RH%c\r", 0x25);
  tft.setCursor(307, 162);
  tft.printf("Chamber TempC\r");
  tft.setCursor(307, 242);
  tft.printf("Chamber RH%c\r", 0x25);
}

// Get X, Y, & pressure values
void readTS(){
  if (digitalRead(TSC_IRQ)){   // IRQ pin is high, nothing to read!
    return;
  }
  p = ts.getPoint();
  //Scale from ~0->4000 to tft.width using the calibration #'s
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.width());
  p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.height());
  if(p.z > 5){
    //Serial.printf("X: %i\nY: %i\nPressure: %i\n", p.x, p.y, p.z);
    //Green path 
    if((p.y > 0) && (p.y < 80)){
      if((p.x > 160) && (p.x < 320)){
        //Serial.printf("Opening  all solenoids\n");
        digitalWrite(SOLENOID_1PIN, HIGH);
        digitalWrite(SOLENOID_2PIN, HIGH);
        digitalWrite(SOLENOID_3PIN, HIGH);
        //delay(5000);
      }
      // Red path
      else if((p.x > 0) && (p.x < 160)){
        //Serial.printf("Closing all solenoids\n");
        digitalWrite(SOLENOID_1PIN, LOW);
        digitalWrite(SOLENOID_2PIN, LOW);
        digitalWrite(SOLENOID_3PIN, LOW);
        //delay(5000);
      }
    }
  }
}


void get_HDC_T_H(float *base_T, double *base_RH, float *chamber_T, double *chamber_RH){
  double baseTemp, chamberTemp;

  base_T_H.readTemperatureHumidityOnDemand(baseTemp, *base_RH,TRIGGERMODE_LP0);
  chamber_T_H.readTemperatureHumidityOnDemand(chamberTemp, *chamber_RH,TRIGGERMODE_LP0);
  *base_T = baseTemp ;
  *chamber_T = chamberTemp;
  //Serial.printf("Base Temp: %0.1f\nBase RH: %0.1f\nChamber Temp: %0.1f\nChamber RH: %0.1f\n", *base_T, *base_RH, *chamber_T, *chamber_RH);
  
}

float getThermoTemp(){
  int reading;
  float voltage;
  float tempC;

  reading = analogRead(TC_PIN);
  voltage = intoVolts(reading);
  tempC = (voltage - 1.25) / 0.005;
  return tempC;
}
  
float getCO2(){
  int measuredBits;
  float measuredVolts;
  float co2Concentration;

  measuredBits = analogRead(LICORINPUTPIN);
  measuredVolts = intoVolts(measuredBits);
  co2Concentration = 2000*(measuredVolts/5.0);
  return co2Concentration;
}

void display_T_H(float bTemperature, float cTemperature, double bHum, double cHum){
  float lastBTemp;
  double lastBHum;
  float lastCTemp;
  double lastCHum;

  get_HDC_T_H(&baseTempReading, &baseRHReading, &chamberTempReading, &chamberRHReading); //Returns the Base & the Chamber Temp+Hum
  tft.setTextSize(3);
  if(lastBTemp != bTemperature){
    tft.fillRect(135,202,108,24, HX8357_BLACK);
    lastBTemp = bTemperature;
  }
  tft.setCursor(135, 202);
  tft.printf("%0.1f\r", bTemperature);
  
  if(lastBHum != bHum){
    tft.fillRect(135,282,108,24, HX8357_BLACK);     
    lastBHum = bHum;
  }
  tft.setCursor(135, 282);
  tft.printf("%0.1f\r", bHum);
  
  if(lastCTemp != cTemperature){
    tft.fillRect(320,202,108,24, HX8357_BLACK);
    lastCTemp = cTemperature;
  }
  tft.setCursor(320, 202);
  tft.printf("%0.1f\r", cTemperature);
  
  if(lastCHum != cHum){
    tft.fillRect(320,282,108,24, HX8357_BLACK); 
    lastCHum = cHum;
  }
  tft.setCursor(320, 282);
  tft.printf("%0.1f\r", cHum);
}

void displayLeafData(float co2, float lux, float leaftTemp){
  tft.setTextSize(3);

  tft.fillRect(115, 80, 95, 24, HX8357_BLACK);
  tft.setCursor(115, 80);
  tft.printf("%0.1f\r", co2);

  
  tft.fillRect(230, 80, 95, 24, HX8357_BLACK);
  tft.setCursor(230, 80);
  tft.printf("%0.1f\r", lux);

  
  tft.fillRect(365, 80, 95, 24, HX8357_BLACK);
  tft.setCursor(365, 80);
  tft.printf("%0.1f\r", leaftTemp);
}

float getLux(){
  return luxSensor.readLux();
}

float intoVolts(int bits){
  float result;

  result = (3.3/4095) * bits;
  return result;
}

