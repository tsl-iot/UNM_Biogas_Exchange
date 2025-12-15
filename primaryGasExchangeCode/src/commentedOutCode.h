#ifndef _COMMENTEDOUTCODE_H
#define _COMMENTEDOUTCODE_H


// Get wavelength 415nm - 680nm readings
// void getAS7341Readings(uint16_t *readingsArray, float *countsArray){
  
//     if(!as7341.readAllChannels(readingsArray)){
//       Serial.printf("Error reading channels\n");
//       return;
//     }
//     for(int i = 0; i < 12; i++){
//       if(i == 4 || i == 5){
//         continue; 
//       }
//       countsArray[i] = as7341.toBasicCounts(readingsArray[i]);
//     }
//     Serial.printf("415nm: %f\n445nm: %f\n480nm: %f\n515nm: %f\n555nm: %f\n590nm: %f\n630nm: %f\n680nm: %f\n", countsArray[0], countsArray[1],countsArray[2], countsArray[3], countsArray[6], countsArray[7], countsArray[8], countsArray[9]);
//   }
  
// void nmToBars(float * basicCounts){
//   float graphBars[12];
//   int pixelSpacing = 48;
//   int barWidth = 40;
//   String xAxisLabels[] = {"415nm", "445nm", "480nm", "515nm", "555nm", "590nm", "630nm", "680nm"};
//   //char yAxisLabels[] = {}
//   //int xAxisLabelSpacing = 
//   int min_X_Pos = 94;
  

//   tft.setTextSize(1);
//   for(int k = 0; k < 8; k++){
//     tft.setCursor(min_X_Pos, 151);
//     tft.printf("%s", xAxisLabels[k].c_str());
//     min_X_Pos = min_X_Pos + pixelSpacing;
//   }
//   min_X_Pos = 94;
//   for(int e = 0; e < 10; e++){
//     if(e == 4 || e == 5){
//       continue;
//     }
//     tft.setCursor(min_X_Pos - 5, 1);
//     tft.fillRect(min_X_Pos - 5, 1, barWidth, 10, HX8357_BLACK);
//     tft.printf("%0.2f", basicCounts[e]);
//     min_X_Pos = min_X_Pos + pixelSpacing;
//   }
//   //min_X_Pos = 94;
//   for(int i = 0; i < 10; i++){
//     if(i == 4 || i == 5){
//       continue;
//     }
//     graphBars[i] = mapFloat(basicCounts[i], 0.0, 5.0, 0.0, 130.0);
//     //Serial.printf("%f\n", graphBars[i]);
//   }


//   tft.fillRect(84, 11, barWidth, 140, HX8357_BLACK);
//   tft.fillRect(84, (140 - graphBars[0]), barWidth, graphBars[0], HX8357_VIOLET);

//   tft.fillRect(84 + pixelSpacing, 11, barWidth, 140, HX8357_BLACK);
//   tft.fillRect(84 + pixelSpacing, (140 - graphBars[1]) , barWidth, graphBars[1], HX8357_BLUE);

//   tft.fillRect(84 + (pixelSpacing*2), 11, barWidth, 140, HX8357_BLACK);
//   tft.fillRect(84 + (pixelSpacing*2), (140 - graphBars[2]), barWidth, graphBars[2], HX8357_CYAN);

//   tft.fillRect(84 + (pixelSpacing*3), 11, barWidth, 140, HX8357_BLACK);
//   tft.fillRect(84 + (pixelSpacing*3), (140 - graphBars[3]), barWidth, graphBars[3], HX8357_GREEN);

//   tft.fillRect(84 + (pixelSpacing*4), 11, barWidth, 140, HX8357_BLACK);
//   tft.fillRect(84 + (pixelSpacing*4), (140 - graphBars[6]), barWidth, graphBars[6], HX8357_YELLOW);
  
//   tft.fillRect(84 + (pixelSpacing*5), 11, barWidth, 140, HX8357_BLACK);
//   tft.fillRect(84 + (pixelSpacing*5), (140 - graphBars[7]), barWidth, graphBars[7], HX8357_ORANGE);  

//   tft.fillRect(84 + (pixelSpacing*6), 11, barWidth, 140, HX8357_BLACK);
//   tft.fillRect(84 + (pixelSpacing*6), (140 - graphBars[8]), barWidth, graphBars[8], HX8357_ORANGE);  

//   tft.fillRect(84 + (pixelSpacing*7), 11, barWidth, 140, HX8357_BLACK);
//   tft.fillRect(84 + (pixelSpacing*7), (140 - graphBars[9]), barWidth, graphBars[9], HX8357_RED);    
// }


// float mapFloat(float value, float inMin, float inMax, float outMin, float outMax) {
//   float newValue;

//   newValue = value * ((outMax-outMin)/(inMax-inMin)) + outMin;
//   return newValue;
// }

// void displaySolenoidControls(){
//   int buttonRad = 35;

// tft.drawCircle(347, 205, buttonRad, HX8357_WHITE);
// tft.setCursor(325, 185);
// tft.printf("S1");
// tft.drawCircle(274, 265, buttonRad, HX8357_WHITE);
// tft.setCursor(252, 245);
// tft.printf("S2");
// tft.drawCircle(423, 265, buttonRad, HX8357_WHITE);
// tft.setCursor(401, 245);
// tft.printf("S3");
// }


  // uint8_t fault = leafTC.readFault();
  // if (fault) {
  //   if (fault & MAX31856_FAULT_CJRANGE) Serial.println("Cold Junction Range Fault");
  //   if (fault & MAX31856_FAULT_TCRANGE) Serial.println("Thermocouple Range Fault");
  //   if (fault & MAX31856_FAULT_CJHIGH)  Serial.println("Cold Junction High Fault");
  //   if (fault & MAX31856_FAULT_CJLOW)   Serial.println("Cold Junction Low Fault");
  //   if (fault & MAX31856_FAULT_TCHIGH)  Serial.println("Thermocouple High Fault");
  //   if (fault & MAX31856_FAULT_TCLOW)   Serial.println("Thermocouple Low Fault");
  //   if (fault & MAX31856_FAULT_OVUV)    Serial.println("Over/Under Voltage Fault");
  //   if (fault & MAX31856_FAULT_OPEN)    Serial.println("Thermocouple Open Fault");
  //   return 0.0;
  // }
  //else{
    // Serial.printf("\n");
    // Serial.println("Current temp:");
    // Serial.println(tempF);
    // Serial.printf("\n");
#endif //commentedOutCode_h