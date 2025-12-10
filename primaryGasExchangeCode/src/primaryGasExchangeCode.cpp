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

#include <Adafruit_MAX31856.h>  //added for new TC - JPP
#include "IoTClassroom_CNM.h"
#include "Adafruit_VEML7700.h"

#include "../lib/Adafruit_GFX_RK/src/Adafruit_GFX_RK.h"
#include "../lib/Adafruit_HX8357_RK/src/Adafruit_HX8357_RK.h"
#include "../lib/Adafruit_TSC2007/src/Adafruit_TSC2007.h"
#include "../lib/Adafruit_BusIO_Register/src/Adafruit_BusIO_Register.h"
#include "../lib/Adafruit_HDC302x/src/Adafruit_HDC302x.h"
#include "../lib/Adafruit_VEML7700/src/Adafruit_VEML7700.h"
#include "../lib/Adafruit_MAX31856_library/src/Adafruit_MAX31856.h"

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
void readTS();
void layoutHomeScreen();

void initSolenoidValves(const int S1_PIN, const int S2_PIN, const int S3_PIN);
//void calibrateTherm(); commented out for thermocouple upgrade  - JPP


void get_HDC_T_H(float *base_T, double *base_RH, float *chamber_T, double *chamber_RH);
void display_T_H(float bTemperature, float cTemperature, double bHum, double cHum);
void displayLeafData(float co2, float lux, float leaftTemp);

float getThermoTemp();
int getCO2();
float intoVolts(int bits);
void initVEML7700();
float getLux();

// Class Objects
Adafruit_HDC302x base_T_H = Adafruit_HDC302x();
Adafruit_HDC302x chamber_T_H = Adafruit_HDC302x();
Adafruit_HX8357 tft(TFT_CS, TFT_DC, TFT_RST);
Adafruit_TSC2007 ts; // newer rev 2 touch contoller
TS_Point p;

//AdafruitMAX31855 chamberThermoCup(THERM_CLK,THERM_CS,THERM_DO);  commented out for TC Upgrade - JPP
Adafruit_MAX31856 leafTC = Adafruit_MAX31856(5, 15, 16, 17); // object for new TC - JPP
Adafruit_VEML7700_ luxSensor;

// Start of the program
void setup() {
  Serial.begin(9600);
  waitFor(Serial.isConnected, 5000);
  hdc302xInit(0x44, 0x47);
  displayInit();
  initVEML7700();
  layoutHomeScreen();
  //chamberThermoCup.init();
  //calibrateTherm(); commented out for TC upgrade - JPP


  initSolenoidValves(SOLENOID_1PIN, SOLENOID_2PIN, SOLENOID_3PIN);


  delay(2000);



  // set up for new TC - JPP
  leafTC.begin();
  leafTC.setThermocoupleType(MAX31856_TCTYPE_K);

}

void loop() {

  readTS();
  if((millis() - lastPrint) > 1000){
    co2Val = getCO2();
    luxReading = getLux();
    leafThermoTemp = getThermoTemp();
    displayLeafData(co2Val, luxReading, leafThermoTemp);
    display_T_H(baseTempReading, chamberTempReading, baseRHReading, chamberRHReading);
    
    
    //Serial.printf("Base Temp: %0.1f\nBase RH: %0.1f\nChamber Temp: %0.1f\nChamber RH: %0.1f\nleaf temp: %0.1f\n", baseTempReading, baseRHReading, chamberTempReading, chamberRHReading, leafThermoTemp);
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

// Sets pinModes and initializes each solenoid to the LOW mode
void initSolenoidValves(const int S1_PIN, const int S2_PIN, const int S3_PIN){
  pinMode(S1_PIN, OUTPUT);
  pinMode(S2_PIN, OUTPUT);
  pinMode(S3_PIN, OUTPUT);

  digitalWrite(S1_PIN, LOW);
  digitalWrite(S2_PIN, LOW);
  digitalWrite(S3_PIN, LOW);
}

// Thermo Calibration function   commented out for thermocouple update - JPP
// void calibrateTherm(){ // can either be called with "calTemp:XX" where xx is the current temp in Celsius or with no argument to calibrate to internal temp sensor
 
//     chamberThermoCup.calibrate();
 
// }

// Get X, Y, & pressure values
void readTS(){
  // int buttonRad = 35;
  // static bool s1_toggle;
  // static bool s2_toggle;
  // static bool s3_toggle;

  // static bool lastS1Toggle;
  // static bool lastS2Toggle;
  // static bool lastS3Toggle;

  // static bool s1Clicked;
  // static bool s2Clicked;
  // static bool s3Clicked;
  static int which_TH;




  tft.setRotation(1); // Landscape
  
 
  p = ts.getPoint();
  
  
  //Scale from ~0->4000 to tft.width using the calibration #'s
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.width());
  p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.height());
  // p.x = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
  // p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());

  Serial.printf("X: %i\nY: %i\nPressure: %i\n", p.x, p.y, p.z);
  
  

  
  
  
  //   if((p.y > 15) && (p.y < 65)){
  //     if((p.x > 240) && (p.x < 290)){
      
  //     which_TH = 1;
  //   }
  //   if((p.x > 130) && (p.x < 180)){
      
  //     which_TH = 2;
  //   }
  //   if((p.x > 30) && (p.x < 80)){
      
  //     which_TH = 3;
  //   }
  // }


  
   

//   lastS1Toggle = s1_toggle;
//   if((p.x > 80) && (p.x < 150) && (p.y > 320)&& (p.y <400)){
//     s1_toggle = true;
//     if(s1_toggle != lastS1Toggle){
//     s1Clicked = !s1Clicked;
//   }
//   s1_toggle = false;
// }
//     if(s1Clicked){
//       tft.fillCircle(347, 205, buttonRad, HX8357_GREEN);
//       tft.setCursor(325, 185);
//       tft.printf("S1");
//       digitalWrite(SOLENOID_1PIN, HIGH);
//     }
//     else{
//       tft.fillCircle(347, 205, buttonRad, HX8357_RED);
//       tft.setCursor(325, 185);
//       tft.printf("S1");
//       digitalWrite(SOLENOID_1PIN, LOW);
//     }

 
//     lastS2Toggle = s2_toggle;
//   if((p.x > 20) && (p.x < 90) && (p.y > 245)&& (p.y <320)){
//     s2_toggle = true;
//     if(s2_toggle != lastS2Toggle){
//     s2Clicked = !s2Clicked;
//   }
//   s2_toggle = false;
// }
//     if(s2Clicked){
//       tft.fillCircle(274, 265, buttonRad, HX8357_GREEN);
//       tft.setCursor(252, 245);
//       tft.printf("S2");
//       digitalWrite(SOLENOID_2PIN, HIGH);
//     }
//     else{
//       tft.fillCircle(274, 265, buttonRad, HX8357_RED);
//       tft.setCursor(252, 245);
//       tft.printf("S2");
//       digitalWrite(SOLENOID_2PIN, LOW);
//     }
  
// lastS3Toggle = s3_toggle;
//   if((p.x > 25) && (p.x < 100) && (p.y > 400)&& (p.y <475)){
// s3_toggle = true;
//     if(s3_toggle != lastS3Toggle){
//     s3Clicked = !s3Clicked;
//   }
//   s3_toggle = false;
//   }
//     if(s3Clicked){
//       tft.fillCircle(423, 265, buttonRad, HX8357_GREEN);
//       tft.setCursor(401, 245);
//       tft.printf("S3");
//       digitalWrite(SOLENOID_3PIN, HIGH);
//     }
//     else{
//       tft.fillCircle(423, 265, buttonRad, HX8357_RED);
//       tft.setCursor(401, 245);
//       tft.printf("S3");
//       digitalWrite(SOLENOID_3PIN, LOW);
//     }

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
  tft.setCursor(250, 12);
  tft.printf("Lux");
  tft.setTextSize(2); 
  tft.setCursor(355, 12);
  tft.printf("Leaf TempF");

  tft.setTextSize(2);
  tft.setCursor(117, 162);
  tft.printf("Base TempF\r");
  tft.setCursor(117, 242);
  tft.printf("Base RH%c\r", 0x25);
  tft.setCursor(307, 162);
  tft.printf("Chamber TempF\r");
  tft.setCursor(307, 242);
  tft.printf("Chamber RH%c\r", 0x25);
}



void get_HDC_T_H(float *base_T, double *base_RH, float *chamber_T, double *chamber_RH){
  double baseTemp, chamberTemp;

  base_T_H.readTemperatureHumidityOnDemand(baseTemp, *base_RH,TRIGGERMODE_LP0);
  chamber_T_H.readTemperatureHumidityOnDemand(chamberTemp, *chamber_RH,TRIGGERMODE_LP0);
  *base_T = (baseTemp * (9.0/5.0)) + 32;
  *chamber_T = (chamberTemp * (9.0/5.0)) + 32;
  //Serial.printf("Base Temp: %0.1f\nBase RH: %0.1f\nChamber Temp: %0.1f\nChamber RH: %0.1f\n", *base_T, *base_RH, *chamber_T, *chamber_RH);
  
}

void display_T_H(float bTemperature, float cTemperature, double bHum, double cHum){
  get_HDC_T_H(&baseTempReading, &baseRHReading, &chamberTempReading, &chamberRHReading); //Returns the Base & the Chamber Temp+Hum
  tft.setTextSize(3);
  //tft.drawRect(77,160,140,160,HX8357_WHITE);

  //tft.fillRect(76,160,(DISPLAY_W-76)/2,160, HX8357_VIOLET);
  tft.setCursor(123, 202);
  tft.fillRect(123,202,108,24, HX8357_BLACK); // (480 - 76)/3 = 135 - 76 = 59 + 76 = = 106
  tft.printf("%0.2f\r", bTemperature);

  // tft.setCursor(250, 12);
  // tft.printf("Lux");
  // tft.fillRect(251, 80, 72, 18, HX8357_BLACK);
  // tft.setCursor(251, 80);
  // tft.printf("%i\r", bHum);

  // tft.setTextSize(2); 
  // tft.setCursor(355, 12);
  // tft.printf("Leaf TempF");
  // tft.setTextSize(3);
  // tft.fillRect(375, 80, 72, 18, HX8357_BLACK);
  // tft.setCursor(375, 80);
  // tft.printf("%0.2f\r", cTemperature);

  // tft.setCursor(304, 250);
  // tft.printf("Chamber RH");
  // tft.setCursor(350, 285);
  // tft.printf("%0.1f\r", bHum);


  


  // tft.setCursor(80, 285);
  // tft.printf("Base RH: %0.1f", bHum);

  // tft.fillRect(278,160,(DISPLAY_W-76)/2,160, HX8357_INDIGO);

  // tft.setCursor(282, 200);
  // tft.printf("Chamber TempF: %0.1f", cTemperature);
  // tft.setCursor(282, 280);
  // tft.printf("Chamber RH: %0.1f", cHum);



  
 
  // tft.setCursor(147,225);
  // tft.fillRect(147, 225, 60, 16, HX8357_BLACK);
  // tft.printf("%0.1fF", temperature);
  // tft.setCursor(147, 291);
  // tft.fillRect(147, 291, 60, 16, HX8357_BLACK);  
  // tft.printf("%i", (int)humOrCO2);
}



float getThermoTemp(){
  double tempC = leafTC.readThermocoupleTemperature();
  double tempF = tempC * (9.0/5.0)  + 32.0;
  uint8_t fault = leafTC.readFault();
  if (fault) {
    if (fault & MAX31856_FAULT_CJRANGE) Serial.println("Cold Junction Range Fault");
    if (fault & MAX31856_FAULT_TCRANGE) Serial.println("Thermocouple Range Fault");
    if (fault & MAX31856_FAULT_CJHIGH)  Serial.println("Cold Junction High Fault");
    if (fault & MAX31856_FAULT_CJLOW)   Serial.println("Cold Junction Low Fault");
    if (fault & MAX31856_FAULT_TCHIGH)  Serial.println("Thermocouple High Fault");
    if (fault & MAX31856_FAULT_TCLOW)   Serial.println("Thermocouple Low Fault");
    if (fault & MAX31856_FAULT_OVUV)    Serial.println("Over/Under Voltage Fault");
    if (fault & MAX31856_FAULT_OPEN)    Serial.println("Thermocouple Open Fault");
    return 0.0;
  }else{
    Serial.printf("\n");
    Serial.println("Current temp:");
    Serial.println(tempF);
    Serial.printf("\n");
    return tempF;
    }
  


  
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

void displayLeafData(float co2, float lux, float leaftTemp){
  tft.setTextSize(3);

  tft.fillRect(105, 80, 90, 24, HX8357_BLACK);
  tft.setCursor(105, 80);
  tft.printf("%0.2f\r", co2);

  
  tft.fillRect(230, 80, 90, 24, HX8357_BLACK);
  tft.setCursor(230, 80);
  tft.printf("%0.1f\r", lux);

  
  tft.fillRect(375, 80, 90, 24, HX8357_BLACK);
  tft.setCursor(375, 80);
  tft.printf("%0.2f\r", leaftTemp);

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

float getLux(){
  return luxSensor.readLux();
}
