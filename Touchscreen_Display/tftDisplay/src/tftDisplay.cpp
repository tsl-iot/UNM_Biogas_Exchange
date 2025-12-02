/* 
 * Project myProject
 * Author: Your Name
 * Date: 
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"
#include <Adafruit_GFX.h>
#include <Adafruit_HX8357.h>
#include "Adafruit_TSC2007.h"
#include <Adafruit_HDC302x.h>

#include "../lib/Adafruit_GFX_RK/src/Adafruit_GFX_RK.h"
#include "../lib/Adafruit_HX8357_RK/src/Adafruit_HX8357_RK.h"
#include "../lib/Adafruit_TSC2007/src/Adafruit_TSC2007.h"
#include "../lib/Adafruit_BusIO_RK/src/Adafruit_BusIO_Register.h"
#include "../lib/Adafruit_HDC302x/src/Adafruit_HDC302x.h"



SYSTEM_MODE(SEMI_AUTOMATIC);

const int TFT_DC = D5;
const int TFT_CS = D4;
const int STMPE_CS = D3;
const int SD_CS = D2;
const int TFT_RST = -1;

// Init screen on hardware SPI, HX8357D type:
Adafruit_HX8357 tft(TFT_CS, TFT_DC, TFT_RST);

// If you're using the TSC2007 there is no CS pin needed, so instead its an IRQ!
const int TSC_IRQ = STMPE_CS;
Adafruit_TSC2007 ts; // newer rev 2 touch contoller

// we will assign the calibration values on init
const int TSC_TS_MINX = 300;
const int TSC_TS_MAXX = 3800;
const int TSC_TS_MINY = 185;
const int TSC_TS_MAXY = 3700;

int16_t min_x, max_x, min_y, max_y;

// Size of the color selection boxes and the paintbrush size
const int BOXWIDTH = 152;
const int BOXHEIGHT = 232;
const int PENRADIUS = 3;
uint16_t oldcolor, currentcolor;
<<<<<<< HEAD
double baseTemp;
float baseTempF;
double postChamberTemp;
float postChamberTempF;
double baseRH;
double postChamberRH;

Adafruit_HDC302x baseBME = Adafruit_HDC302x();
Adafruit_HDC302x chamberBME = Adafruit_HDC302x();


=======
int menuSelection;
int lastSelection;
>>>>>>> 258ecc1900011b6acb8cd3e146c11b8d504a109e

int startMenu(TS_Point pos, const int _BOXWIDTH, const int _BOXHEIGHT);
  void setup() {
  Serial.begin(9600);
  //while (!Serial) delay(10);
  Serial.println("HX8357D Featherwing touch test!");
  if (!ts.begin(0x48)) {
    Serial.printf("Couldn't start TSC2007 touchscreen controller");
    while (1) 
    delay(100);
  }
   if (!baseBME.begin(0x44)) {
    Serial.printf("Could not find sensor?");
    //while (1);
  }
   if (!chamberBME.begin(0x47)) {
    Serial.printf("Could not find sensor?");
    //while (1);
  }


  min_x = TSC_TS_MINX; max_x = TSC_TS_MAXX;
  min_y = TSC_TS_MINY; max_y = TSC_TS_MAXY;
  pinMode(TSC_IRQ, INPUT);
  Serial.printf("Touchscreen started");

  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(HX8357_BLACK);
  // // make the color selection boxes (X,Y, WIDTH, HEIGHT, COLOR)
  // tft.fillRect(4, 4, BOXWIDTH, BOXHEIGHT, HX8357_RED);
  // tft.fillRect(8 + BOXWIDTH, 4, BOXWIDTH, BOXHEIGHT, HX8357_YELLOW);
  // tft.fillRect(4, 8 + BOXHEIGHT, BOXWIDTH, BOXHEIGHT, HX8357_GREEN);
  // tft.fillRect(8 + BOXWIDTH, 8 + BOXHEIGHT, BOXWIDTH, BOXHEIGHT, HX8357_CYAN);
  for(int i = 0; i < 480; i = i + (480/3)){
    tft.drawRect(i,0,(480/3.0),320,HX8357_WHITE);
  }
  tft.drawRect(0,0,480,40,HX8357_WHITE);


  tft.setTextColor(HX8357_BLACK);
  tft.setTextSize(2);

  tft.setCursor(30,120);
  tft.printf("Option 1");

  tft.setCursor(180,120);
  tft.printf("Option 2");

  tft.setCursor(30,360);
  tft.printf("Option 3");

  tft.setCursor(180,360);
  tft.printf("Option 4");

//   // select the current color 'red'
// tft.drawRect(0, 0, BOXSIZE, BOXSIZE, HX8357_WHITE);
// currentcolor = HX8357_RED;
}
void loop(void) {


  if (digitalRead(TSC_IRQ)) {
// IRQ pin is high, nothing to read!
    return;
  }
  TS_Point p = ts.getPoint();

  if (((p.x == 0) && (p.y == 0)) || (p.z < 10)){
    return; // no pressure, no touch
  }
// Scale from ~0->4000 to tft.width using the calibration #'s
  p.x = map(p.x, min_x, max_x, 0, tft.width());
  p.y = map(p.y, min_y, max_y, 0, tft.height());
<<<<<<< HEAD
  Serial.printf("X: %i\nY: %i\nPressure: %i\n", p.x, p.y, p.z);
// BME READINGS
  baseBME.readTemperatureHumidityOnDemand(baseTemp, baseRH, TRIGGERMODE_LP0);
  baseTempF = (baseTemp*9.0/5.0) + 32;

  chamberBME.readTemperatureHumidityOnDemand(postChamberTemp, postChamberRH, TRIGGERMODE_LP0);
  postChamberTempF = (postChamberTemp*9.0/5.0) + 32;


=======
  //Serial.printf("X: %i\nY: %i\nPressure: %i\n", p.x, p.y, p.z);
  menuSelection = startMenu(p, BOXWIDTH, BOXHEIGHT);

  if(menuSelection != lastSelection){
    Serial.printf("option selected: %i\n", menuSelection);
    lastSelection = menuSelection;
  }
>>>>>>> 258ecc1900011b6acb8cd3e146c11b8d504a109e

// if (p.y < BOXSIZE) {
//   oldcolor = currentcolor;
//   if (p.x < BOXSIZE) {
//   currentcolor = HX8357_RED;
//   tft.drawRect(0, 0, BOXSIZE, BOXSIZE, HX8357_WHITE);
//   } else if (p.x < BOXSIZE*2) {
//   currentcolor = HX8357_YELLOW;
//   tft.drawRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, HX8357_WHITE);
//   } else if (p.x < BOXSIZE*3) {
//   currentcolor = HX8357_GREEN;
//   tft.drawRect(BOXSIZE*2, 0, BOXSIZE, BOXSIZE, HX8357_WHITE);
//   } else if (p.x < BOXSIZE*4) {
//   currentcolor = HX8357_CYAN;
//   tft.drawRect(BOXSIZE*3, 0, BOXSIZE, BOXSIZE, HX8357_WHITE);
//   } else if (p.x < BOXSIZE*5) {
//   currentcolor = HX8357_BLUE;
//   tft.drawRect(BOXSIZE*4, 0, BOXSIZE, BOXSIZE, HX8357_WHITE);
//   } else if (p.x < BOXSIZE*6) {
//   currentcolor = HX8357_MAGENTA;
//   tft.drawRect(BOXSIZE*5, 0, BOXSIZE, BOXSIZE, HX8357_WHITE);
//   } else if (p.x < BOXSIZE*7) {
//   currentcolor = HX8357_WHITE;
//   tft.drawRect(BOXSIZE*6, 0, BOXSIZE, BOXSIZE, HX8357_RED);
//   } else if (p.x < BOXSIZE*8) {
//   currentcolor = HX8357_BLACK;
//   tft.drawRect(BOXSIZE*7, 0, BOXSIZE, BOXSIZE, HX8357_WHITE);
//   }
//   if (oldcolor != currentcolor) {
//   if (oldcolor == HX8357_RED)
//   tft.fillRect(0, 0, BOXSIZE, BOXSIZE, HX8357_RED);
//   if (oldcolor == HX8357_YELLOW)
//   tft.fillRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, HX8357_YELLOW);
//   if (oldcolor == HX8357_GREEN)
//   tft.fillRect(BOXSIZE*2, 0, BOXSIZE, BOXSIZE, HX8357_GREEN);
//   if (oldcolor == HX8357_CYAN)
//   tft.fillRect(BOXSIZE*3, 0, BOXSIZE, BOXSIZE, HX8357_CYAN);
//   if (oldcolor == HX8357_BLUE)
//   tft.fillRect(BOXSIZE*4, 0, BOXSIZE, BOXSIZE, HX8357_BLUE);
//   if (oldcolor == HX8357_MAGENTA)
//   tft.fillRect(BOXSIZE*5, 0, BOXSIZE, BOXSIZE, HX8357_MAGENTA);
//   if (oldcolor == HX8357_WHITE)
// tft.fillRect(BOXSIZE*6, 0, BOXSIZE, BOXSIZE, HX8357_WHITE);
// if (oldcolor == HX8357_BLACK)
// tft.fillRect(BOXSIZE*7, 0, BOXSIZE, BOXSIZE, HX8357_BLACK);
// }
// }
// if (((p.y-PENRADIUS) > 0) && ((p.y+PENRADIUS) < tft.height())) {
// tft.fillCircle(p.x, p.y, PENRADIUS, currentcolor);
// }
}

int startMenu(TS_Point pos, const int _BOXWIDTH, const int _BOXHEIGHT){
  static int currentSelection;

  if((pos.x < _BOXWIDTH) && (pos.y < _BOXHEIGHT)){
    //Serial.printf("Quadrant 1 selected\n");
    currentSelection = 1;
    tft.drawRect(4, 4, BOXWIDTH, BOXHEIGHT, HX8357_WHITE);
  }
  else if((pos.x > _BOXWIDTH) && (pos.y < _BOXHEIGHT)){
    //Serial.printf("Quadrant 2 selected\n");
    currentSelection = 2;
    tft.drawRect(8 + BOXWIDTH, 4, BOXWIDTH, BOXHEIGHT, HX8357_WHITE);
  }
  else if((pos.x < _BOXWIDTH) && (pos.y> _BOXHEIGHT)){
    //Serial.printf("Quadrant 3 selected\n");
    currentSelection = 3;
    tft.drawRect(4, 8 + BOXHEIGHT, BOXWIDTH, BOXHEIGHT, HX8357_WHITE);
  }
  else if((pos.x > _BOXWIDTH) && (pos.y > _BOXHEIGHT)){
    //Serial.printf("Quadrant 4 selected\n");
    currentSelection = 4;
    tft.drawRect(8 + BOXWIDTH, 8 + BOXHEIGHT, BOXWIDTH, BOXHEIGHT, HX8357_WHITE);
  }
  return currentSelection;
}

