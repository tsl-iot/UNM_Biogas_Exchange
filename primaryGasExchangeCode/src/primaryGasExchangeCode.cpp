/* 
 * Project Main code that integrates temp/hum, and light sensors. Controls air flow by actuating solenoids. Used by UNM Bio Dept. 
 * Author: Edward Ishman
 * Date: 
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

#include "Particle.h"
#include <Adafruit_GFX.h>
#include <Adafruit_HX8357.h>
#include "Adafruit_TSC2007.h"
#include <Adafruit_HDC302x.h>
#include <Adafruit_AS7341.h>
#include "adafruit-max31855.h"
#include "IoTClassroom_CNM.h"



#include "../lib/Adafruit_GFX_RK/src/Adafruit_GFX_RK.h"
#include "../lib/Adafruit_HX8357_RK/src/Adafruit_HX8357_RK.h"
#include "../lib/Adafruit_TSC2007/src/Adafruit_TSC2007.h"
#include "../lib/Adafruit_BusIO_Register/src/Adafruit_BusIO_Register.h"
#include "../lib/Adafruit_HDC302x/src/Adafruit_HDC302x.h"
#include "../lib/Adafruit_AS7341/src/Adafruit_AS7341.h"
SYSTEM_MODE(SEMI_AUTOMATIC);

// Constants
const int TFT_DC = D5;
const int TFT_CS = D4;
const int STMPE_CS = D3;
const int SD_CS = D2;
const int TFT_RST = -1;
const int TSC_IRQ = STMPE_CS;

const int TS_MINX = 100;
const int TS_MAXX = 3800;
const int TS_MINY = 100;
const int TS_MAXY = 3750;

const int DISPLAY_W = 480;
const int DISPLAY_H = 320;

const int THERM_CLK = A0;
const int THERM_CS = A1;
const int THERM_DO = A2;

const int SOLENOID_1PIN = D6;
const int SOLENOID_2PIN = D10;
const int SOLENOID_3PIN = D19;

const int LICORINPUTPIN = A5;

// Variables
int16_t min_x, max_x, min_y, max_y;
uint16_t as7341Readings[12];
float as7341Counts[12];
float as7341ConvertedCounts[8];
static unsigned int lastPrint;

float baseTempReading;
double baseRHReading;
float chamberTempReading;
double chamberRHReading;
float leafThermoTemp;





// Function Declarations
void hdc302xInit(int address_1, int address_2);
void displayInit();
void readTS();
void layoutHomeScreen();
void initAS7341(uint8_t ATIME, uint16_t ASTEP, as7341_gain_t gainFactor);
void initSolenoidValves(const int S1_PIN, const int S2_PIN, const int S3_PIN);
void calibrateTherm();
void getAS7341Readings(uint16_t *readingsArray, float *countsArray);
float mapFloat(float value, float inMin, float inMax, float outMin, float outMax);
void nmToBars(float * basicCounts);
void get_HDC_T_H(float *base_T, double *base_RH, float *chamber_T, double *chamber_RH);
void display_T_H(float temperature, double humOrCO2, String sensorLabel, bool rHum);
void displaySolenoidControls();
float getThermoTemp();
int getCO2();
float intoVolts(int bits);


// Class Objects
Adafruit_HDC302x base_T_H = Adafruit_HDC302x();
Adafruit_HDC302x chamber_T_H = Adafruit_HDC302x();
Adafruit_HX8357 tft(TFT_CS, TFT_DC, TFT_RST);
Adafruit_TSC2007 ts; // newer rev 2 touch contoller
TS_Point p;
Adafruit_AS7341 as7341;
AdafruitMAX31855 chamberThermoCup(THERM_CLK,THERM_CS,THERM_DO);  //software mode

// Start of the program
void setup() {
  Serial.begin(9600);
  waitFor(Serial.isConnected, 5000);
  Wire.begin();
  hdc302xInit(0x44, 0x47);
  displayInit();
  layoutHomeScreen();
  chamberThermoCup.init();
  calibrateTherm();

  initAS7341(100,999,AS7341_GAIN_64X);
  initSolenoidValves(SOLENOID_1PIN, SOLENOID_2PIN, SOLENOID_3PIN);
  displaySolenoidControls();

  delay(2000);
}


void loop() {

  readTS();
  if((millis() - lastPrint) > 1000){
    getAS7341Readings(as7341Readings, as7341Counts);
    nmToBars(as7341Counts);
    
    leafThermoTemp = getThermoTemp();
    Serial.printf("Base Temp: %0.1f\nBase RH: %0.1f\nChamber Temp: %0.1f\nChamber RH: %0.1f\nleaf temp: %0.1f\n", baseTempReading, baseRHReading, chamberTempReading, chamberRHReading, leafThermoTemp);
    lastPrint = millis();
  }
}

// Use address 0x44 for address_1 and 0x47 for address_2
void hdc302xInit(int baseAddress, int chamberAddress){
  if(!base_T_H.begin(baseAddress)){
    Serial.printf("Could not find Base temp/hum sensor?");
    //while (1);
  }
  if(!chamber_T_H.begin(chamberAddress)){
    Serial.printf("Could not find chamber temp/hum sensor?");
    //while (1);
  }
}

// Use address 0x48 for screenAddress
void displayInit(){
  if (!ts.begin(0x48)) {
    Serial.printf("Couldn't start TSC2007 touchscreen controller\n");
  }
  else{
    Serial.printf("Touchscreen started\n");
  }

  tft.begin();
  tft.setRotation(1);
  min_x = TS_MINX; max_x = TS_MAXX;
  min_y = TS_MINY; max_y = TS_MAXY;
  pinMode(TSC_IRQ, INPUT);
  
// tft object is for the display graphics while ts is for the touch screen portion
}

// Initialize spectral sensor-- ATIME: Sets ADC integration step count -- ASTEP: Integration step size -- AGAIN: Controls ADC Gain --- AS7341_GAIN_0_5X, AS7341_GAIN_1X, AS7341_GAIN_2X, AS7341_GAIN_4X, AS7341_GAIN_8X, AS7341_GAIN_16X, AS7341_GAIN_32X, AS7341_GAIN_64X, AS7341_GAIN_128X, AS7341_GAIN_256X, AS7341_GAIN_512X,
void initAS7341(uint8_t ATIME, uint16_t ASTEP, as7341_gain_t gainFactor){
  if (!as7341.begin()){ // 
    Serial.println("Could not find AS7341");
    while (1) { 
      delay(10); 
    }
  }
    as7341.setATIME(ATIME);
    as7341.setASTEP(ASTEP);
    as7341.setGain(gainFactor);
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

// Thermo Calibration function
void calibrateTherm(){ // can either be called with "calTemp:XX" where xx is the current temp in Celsius or with no argument to calibrate to internal temp sensor
 
    chamberThermoCup.calibrate();
 
}

// Get X, Y, & pressure values
void readTS(){
  int buttonRad = 35;
  static bool s1_toggle;
  static bool s2_toggle;
  static bool s3_toggle;

  static bool lastS1Toggle;
  static bool lastS2Toggle;
  static bool lastS3Toggle;

  static bool s1Clicked;
  static bool s2Clicked;
  static bool s3Clicked;
  static int which_TH;




  tft.setRotation(1); // Landscape
  
 
  p = ts.getPoint();
  
  
  //Scale from ~0->4000 to tft.width using the calibration #'s
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.width());
  p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.height());
  // p.x = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
  // p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());

  Serial.printf("X: %i\nY: %i\nPressure: %i\n", p.x, p.y, p.z);
  
  

  
  
  
    if((p.y > 15) && (p.y < 65)){
      if((p.x > 240) && (p.x < 290)){
      
      which_TH = 1;
    }
    if((p.x > 130) && (p.x < 180)){
      
      which_TH = 2;
    }
    if((p.x > 30) && (p.x < 80)){
      
      which_TH = 3;
    }
  }

  switch (which_TH){
  case 1:
    display_T_H(baseTempReading, baseRHReading, "Base", true);
    break;
  case 2:
    display_T_H(chamberTempReading, chamberRHReading, "Chamber", true);
    break;
  case 3:
    display_T_H(leafThermoTemp, getCO2(), "Leaf", false);
    break;
  default:
    which_TH = 1;
    break;
  }

  lastS1Toggle = s1_toggle;
  if((p.x > 80) && (p.x < 150) && (p.y > 320)&& (p.y <400)){
    s1_toggle = true;
    if(s1_toggle != lastS1Toggle){
    s1Clicked = !s1Clicked;
  }
  s1_toggle = false;
}
    if(s1Clicked){
      tft.fillCircle(347, 205, buttonRad, HX8357_GREEN);
      tft.setCursor(325, 185);
      tft.printf("S1");
      digitalWrite(SOLENOID_1PIN, HIGH);
    }
    else{
      tft.fillCircle(347, 205, buttonRad, HX8357_RED);
      tft.setCursor(325, 185);
      tft.printf("S1");
      digitalWrite(SOLENOID_1PIN, LOW);
    }

 
    lastS2Toggle = s2_toggle;
  if((p.x > 20) && (p.x < 90) && (p.y > 245)&& (p.y <320)){
    s2_toggle = true;
    if(s2_toggle != lastS2Toggle){
    s2Clicked = !s2Clicked;
  }
  s2_toggle = false;
}
    if(s2Clicked){
      tft.fillCircle(274, 265, buttonRad, HX8357_GREEN);
      tft.setCursor(252, 245);
      tft.printf("S2");
      digitalWrite(SOLENOID_2PIN, HIGH);
    }
    else{
      tft.fillCircle(274, 265, buttonRad, HX8357_RED);
      tft.setCursor(252, 245);
      tft.printf("S2");
      digitalWrite(SOLENOID_2PIN, LOW);
    }
  
lastS3Toggle = s3_toggle;
  if((p.x > 25) && (p.x < 100) && (p.y > 400)&& (p.y <475)){
s3_toggle = true;
    if(s3_toggle != lastS3Toggle){
    s3Clicked = !s3Clicked;
  }
  s3_toggle = false;
  }
    if(s3Clicked){
      tft.fillCircle(423, 265, buttonRad, HX8357_GREEN);
      tft.setCursor(401, 245);
      tft.printf("S3");
      digitalWrite(SOLENOID_3PIN, HIGH);
    }
    else{
      tft.fillCircle(423, 265, buttonRad, HX8357_RED);
      tft.setCursor(401, 245);
      tft.printf("S3");
      digitalWrite(SOLENOID_3PIN, LOW);
    }

  //   if(((p.x == 0) && (p.y == 0)) || (p.z < 10)){
  //     return; // no pressure, no touch
  // }
   if (digitalRead(TSC_IRQ)){   // IRQ pin is high, nothing to read!
    return;
  }


}

// Draws the shapes that outlines the home screen
void layoutHomeScreen(){
  int menuRect_W = DISPLAY_W * .16;
  int barGraphBox_W = DISPLAY_W - menuRect_W;
  int temp_hum_box_W = DISPLAY_W - menuRect_W;
  int barGraphBox_H = DISPLAY_H/2;
  int temp_hum_box_H = DISPLAY_H/2;
  int circleRad = 25;
  tft.setTextSize(3);
  tft.setRotation(1); // Landscape
  tft.fillScreen(HX8357_BLACK);
  tft.drawRect(0,0,menuRect_W,DISPLAY_H, HX8357_WHITE); // left panel option menu
  tft.drawRect(76,0,barGraphBox_W,barGraphBox_H, HX8357_WHITE);
  tft.drawRect(76,160,temp_hum_box_W,temp_hum_box_H, HX8357_WHITE);
  tft.fillCircle(38,55,circleRad,HX8357_BLUE);
  tft.setCursor(32, 45);
  tft.printf("B");
  
  tft.fillCircle(38,160,circleRad,HX8357_INDIGO);
  tft.setCursor(32, 150);
  tft.printf("C");

  tft.fillCircle(38,265,circleRad,HX8357_MAGENTA);
  tft.setCursor(32, 255);
  tft.printf("L");
  
}

// Get wavelength 415nm - 680nm readings
void getAS7341Readings(uint16_t *readingsArray, float *countsArray){
  
    if(!as7341.readAllChannels(readingsArray)){
      Serial.printf("Error reading channels\n");
      return;
    }
    for(int i = 0; i < 12; i++){
      if(i == 4 || i == 5){
        continue; 
      }
      countsArray[i] = as7341.toBasicCounts(readingsArray[i]);
    }
    Serial.printf("415nm: %f\n445nm: %f\n480nm: %f\n515nm: %f\n555nm: %f\n590nm: %f\n630nm: %f\n680nm: %f\n", countsArray[0], countsArray[1],countsArray[2], countsArray[3], countsArray[6], countsArray[7], countsArray[8], countsArray[9]);
  }
  
void nmToBars(float * basicCounts){
  float graphBars[12];
  int pixelSpacing = 48;
  int barWidth = 40;
  String xAxisLabels[] = {"415nm", "445nm", "480nm", "515nm", "555nm", "590nm", "630nm", "680nm"};
  //char yAxisLabels[] = {}
  //int xAxisLabelSpacing = 
  int min_X_Pos = 94;
  

  tft.setTextSize(1);
  for(int k = 0; k < 8; k++){
    tft.setCursor(min_X_Pos, 151);
    tft.printf("%s", xAxisLabels[k].c_str());
    min_X_Pos = min_X_Pos + pixelSpacing;
  }
  min_X_Pos = 94;
  for(int e = 0; e < 10; e++){
    if(e == 4 || e == 5){
      continue;
    }
    tft.setCursor(min_X_Pos - 5, 1);
    tft.fillRect(min_X_Pos - 5, 1, barWidth, 10, HX8357_BLACK);
    tft.printf("%0.2f", basicCounts[e]);
    min_X_Pos = min_X_Pos + pixelSpacing;
  }
  //min_X_Pos = 94;
  for(int i = 0; i < 10; i++){
    if(i == 4 || i == 5){
      continue;
    }
    graphBars[i] = mapFloat(basicCounts[i], 0.0, 5.0, 0.0, 130.0);
    //Serial.printf("%f\n", graphBars[i]);
  }


  tft.fillRect(84, 11, barWidth, 140, HX8357_BLACK);
  tft.fillRect(84, (140 - graphBars[0]), barWidth, graphBars[0], HX8357_VIOLET);

  tft.fillRect(84 + pixelSpacing, 11, barWidth, 140, HX8357_BLACK);
  tft.fillRect(84 + pixelSpacing, (140 - graphBars[1]) , barWidth, graphBars[1], HX8357_BLUE);

  tft.fillRect(84 + (pixelSpacing*2), 11, barWidth, 140, HX8357_BLACK);
  tft.fillRect(84 + (pixelSpacing*2), (140 - graphBars[2]), barWidth, graphBars[2], HX8357_CYAN);

  tft.fillRect(84 + (pixelSpacing*3), 11, barWidth, 140, HX8357_BLACK);
  tft.fillRect(84 + (pixelSpacing*3), (140 - graphBars[3]), barWidth, graphBars[3], HX8357_GREEN);

  tft.fillRect(84 + (pixelSpacing*4), 11, barWidth, 140, HX8357_BLACK);
  tft.fillRect(84 + (pixelSpacing*4), (140 - graphBars[6]), barWidth, graphBars[6], HX8357_YELLOW);
  
  tft.fillRect(84 + (pixelSpacing*5), 11, barWidth, 140, HX8357_BLACK);
  tft.fillRect(84 + (pixelSpacing*5), (140 - graphBars[7]), barWidth, graphBars[7], HX8357_ORANGE);  

  tft.fillRect(84 + (pixelSpacing*6), 11, barWidth, 140, HX8357_BLACK);
  tft.fillRect(84 + (pixelSpacing*6), (140 - graphBars[8]), barWidth, graphBars[8], HX8357_ORANGE);  

  tft.fillRect(84 + (pixelSpacing*7), 11, barWidth, 140, HX8357_BLACK);
  tft.fillRect(84 + (pixelSpacing*7), (140 - graphBars[9]), barWidth, graphBars[9], HX8357_RED);    
}

float mapFloat(float value, float inMin, float inMax, float outMin, float outMax) {
  float newValue;

  newValue = value * ((outMax-outMin)/(inMax-inMin)) + outMin;
  return newValue;
}

void get_HDC_T_H(float *base_T, double *base_RH, float *chamber_T, double *chamber_RH){
  double baseTemp, chamberTemp;

  base_T_H.readTemperatureHumidityOnDemand(baseTemp, *base_RH,TRIGGERMODE_LP0);
  chamber_T_H.readTemperatureHumidityOnDemand(chamberTemp, *chamber_RH,TRIGGERMODE_LP0);
  *base_T = (baseTemp * (9.0/5.0)) + 32;
  *chamber_T = (chamberTemp * (9.0/5.0)) + 32;
  //Serial.printf("Base Temp: %0.1f\nBase RH: %0.1f\nChamber Temp: %0.1f\nChamber RH: %0.1f\n", *base_T, *base_RH, *chamber_T, *chamber_RH);
  
}

void display_T_H(float temperature, double humOrCO2, String sensorLabel, bool rHum){
  get_HDC_T_H(&baseTempReading, &baseRHReading, &chamberTempReading, &chamberRHReading);
  tft.setTextSize(2);
  tft.drawRect(77,160,140,160,HX8357_WHITE);
  tft.fillRect(78,164, 135,18,HX8357_BLACK);
  tft.setCursor(120, 165);
  tft.printf("%s", sensorLabel.c_str());
  tft.setCursor(80, 185);
  tft.printf("Temperature");
   tft.fillRect(80,250,130,20, HX8357_BLACK);
  tft.setCursor(80, 250);
  if(rHum){
  tft.printf("RH%c", 0x25);
  }
  else{
    tft.printf("CO2 ppm");
  }
 
  tft.setCursor(147,225);
  tft.fillRect(147, 225, 60, 16, HX8357_BLACK);
  tft.printf("%0.1fF", temperature);
  tft.setCursor(147, 291);
  tft.fillRect(147, 291, 60, 16, HX8357_BLACK);  
  tft.printf("%i", (int)humOrCO2);
}

void displaySolenoidControls(){
  int buttonRad = 35;

tft.drawCircle(347, 205, buttonRad, HX8357_WHITE);
tft.setCursor(325, 185);
tft.printf("S1");
tft.drawCircle(274, 265, buttonRad, HX8357_WHITE);
tft.setCursor(252, 245);
tft.printf("S2");
tft.drawCircle(423, 265, buttonRad, HX8357_WHITE);
tft.setCursor(401, 245);
tft.printf("S3");
}

float getThermoTemp(){
  return chamberThermoCup.readFarenheit();
}

float intoVolts(int bits){
  float result;

  result = (3.3/4095) * bits;
  return result;
}

int getCO2(){
int measuredBits;
float measuredVolts;
int co2Concentration;

  measuredBits = analogRead(A5);
  measuredVolts = intoVolts(measuredBits);
  co2Concentration = 2000*(measuredVolts/5.0);
  return co2Concentration;

}