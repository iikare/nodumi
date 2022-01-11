#include "kmeans.h"
#include "../dpd/CIEDE2000/CIEDE2000.h"
#include "define.h"
#include "log.h"



double kMeansPoint::distance(kMeansPoint& point) {

  // data       -> A
  // point.data -> B

  #ifdef COLDIST_CIE00
  // CIEDE2000 implementation
  // as described in "The CIEDE2000 Color-Difference Formula: Implementation Notes, 
  // Supplementary Test Data, and Mathematical Observations" by Gaurav Sharma, Wencheng Wu, Edul N. Dalal
 
  CIEDE2000::LAB lab1 = {data.l, data.a, data.b};
  CIEDE2000::LAB lab2 = {point.data.l, point.data.a, point.data.b}; 

  return CIEDE2000::CIEDE2000(lab1, lab2);

  //// lch intermediary NOT needed
  //colorLAB lab1(data);
  //colorLAB lab2(point.data);

  //// eq2
  //float C_1 = sqrt(lab1.a*lab1.a + lab1.b*lab1.b);
  //float C_2 = sqrt(lab2.a*lab2.a + lab2.b*lab2.b);

  //// eq3
  //float cBar = (C_1+C_2)/2.0f;

  //const auto p7 = [] (float v) {
    //return v*v*v*v*v*v*v;
  //};

  
  //// eq4
  //float p725 = 6103515625.0f;
  //float G = 0.5f * (1.0f - sqrt(p7(cBar)/(p7(cBar)+p725)));

  //// eq5
  //float aPrime_1 = (1.0f + G) *lab1.a;
  //float aPrime_2 = (1.0f + G) *lab1.b;

  //// eq6
  //float cPrime_1 = sqrt(aPrime_1*aPrime_1 + lab1.b*lab1.b);
  //float cPrime_2 = sqrt(aPrime_2*aPrime_2 + lab2.b*lab2.b);

  //// eq7
  //float hPrime_1, hPrime_2 = 0;
  //if (lab1.b != 0 || aPrime_1 != 0) {
    //hPrime_1 = atan2(lab1.b, aPrime_1) * RAD2DEG;
    //if (hPrime_1 < 0) {
      //hPrime_1 += 360;
    //}
  //}
  //if (lab1.b != 0 || aPrime_1 != 0) {
    //hPrime_2 = atan2(lab2.b, aPrime_2) * RAD2DEG;
    //if (hPrime_2 < 0) {
      //hPrime_2 += 360;
    //}
  //}

  //// eq8
  //float deltaLPrime = lab2.l - lab1.l;
  
  //// eq9
  //float deltaCPrime = cPrime_2 - cPrime_1;

  //// eq10
  //float deltaHPrimeSmall = 0;
  //if (cPrime_1 != 0 || cPrime_2 != 0) {
    //if (fabs(hPrime_2 - hPrime_1) <= 180.0f) {
      //deltaHPrimeSmall = hPrime_2 - hPrime_1;
    //}
    //else {
      //if (hPrime_2 - hPrime_1 > 180.0f) {
        //deltaHPrimeSmall = hPrime_2 - hPrime_1 - 360;
      //}
      //else {
        //deltaHPrimeSmall = hPrime_2 - hPrime_1 + 360;
      //}
    //}
  //}

  //// eq11
  //float deltaHPrimeBig = 0;
  //if (cPrime_1 != 0 || cPrime_2 != 0) {
    //if (deltaHPrimeSmall != 0) {
      //deltaHPrimeBig = 2 * sqrt(cPrime_1*cPrime_2) * sinDeg(deltaHPrimeSmall/2.0f);
    //}
  //}

  //// eq12
  //float lBarPrime = (lab1.l + lab2.l)/2.0f;

  //// eq13
  //float cBarPrime = (cPrime_1 + cPrime_2)/2.0f;

  //// eq14
  //float hBarPrime = 0;

  //if (cPrime_1 == 0 && cPrime_2 == 0) {
    //hBarPrime = hPrime_1 + hPrime_2;
  //}
  //else {
    //float hDiff = fabs(hPrime_1 - hPrime_2);
    //if (hDiff <= 180) {
      //hBarPrime = (hPrime_1+hPrime_2)/2.0f;
    //}
    //else {
      //if (hPrime_1 + hPrime_2 < 360) {
        //hBarPrime = (hPrime_1+hPrime_2+360)/2.0f;
      //}
      //else {
        //hBarPrime = (hPrime_1+hPrime_2-360)/2.0f;
      //}
    //}
  //}

  //// eq15
  //float T = 1 - 0.17*cosDeg(hBarPrime-30) + 0.24*cosDeg(2*hBarPrime) +
                //0.32*cosDeg(3*hBarPrime+6)- 0.20*cosDeg(4*hBarPrime-63);  

  //// eq16
  //float dtExp = (hBarPrime - 275)/25;
  //float deltaTheta = 30*exp(-dtExp*dtExp);

  //// eq17
  //float R_C = 2*sqrt(p7(cBarPrime)/(p7(cBarPrime)+p725));

  //// eq18
  //float lBPExp = lBarPrime-50;
  //float S_L = 1 + 0.015*lBPExp*lBPExp/sqrt(20+lBPExp*lBPExp);

  //// eq19
  //float S_C = 1 + 0.045*cBarPrime;

  //// eq20
  //float S_H = 1 + 0.015*cBarPrime*T;

  //// eq21
  //float R_T = -sinDeg(2*deltaTheta)*R_C;

  //// eq22 +  constants
  
  //constexpr float k_L = 1.0f;
  //constexpr float k_C = 1.0f;
  //constexpr float k_H = 1.0f;

  //// subcomponents
  //float dE_1 = deltaLPrime/(k_L*S_L);
  //float dE_2 = deltaCPrime/(k_C*S_C);
  //float dE_3 = deltaHPrimeBig/(k_H*S_H);
  //float dE_4 = R_T*deltaCPrime*deltaHPrimeBig/(k_C*S_C*k_H*S_H);

  //return sqrt(dE_1*dE_1 + dE_2*dE_2 + dE_3*dE_3 + dE_4);
  //return dE_1*dE_1 + dE_2*dE_2 + dE_3*dE_3 + dE_4;

  

  
  //lchIntermediary pA = {(float)data.l, (float)sqrt(pow(data.a,2) + pow(data.b,2)), 0*(float)atan2(data.b, data.a)};
  //lchIntermediary pB = {(float)point.data.l, (float)sqrt(pow(point.data.a,2) + pow(point.data.b,2)), 
                        //0*(float)atan2(point.data.b, point.data.a)};
  
  //float deltaLPrime = pB.l - pA.l;

  //float lBar = (pA.l + pB.l)/2.0f;
  //float cBar = (pA.c + pB.c)/2.0f;

  //float cBarIntermediary = sqrt(pow(cBar,7)/(pow(cBar,7)+pow(25,7))); 
  //float aPrime_1 = data.a + data.a/2.0f * (1 - cBarIntermediary);
  //float aPrime_2 = point.data.a + point.data.a/2.0f * (1 - cBarIntermediary); 

  //float cPrime_1 = sqrt(pow(aPrime_1, 2) + pow(data.b, 2));
  //float cPrime_2 = sqrt(pow(aPrime_2, 2) + pow(point.data.b, 2));

  //float deltaCPrime = cPrime_2 - cPrime_1;
  //float cBarPrime = (cPrime_1 + cPrime_2) / 2.0f;
  //float cBarPrimeIntermediary = sqrt(pow(cBarPrime,7)/(pow(cBarPrime,7)+pow(25,7))); 
  
  //float hPrime_1 = 0;
  //if (data.b != 0 || aPrime_1 != 0) {
    //hPrime_1 = fmod(atan2f(data.b, aPrime_1) * RAD2DEG + 360.0f, 360.0f);
    ////logQ("1:",hPrime_1);
  //}
  
  //float hPrime_2 = 0;
  //if (point.data.b != 0 || aPrime_2 != 0) {
    //hPrime_2 = fmod(atan2f(point.data.b, aPrime_2) * RAD2DEG + 360.0f, 360.0f);
    ////logQ("2:",hPrime_2);
  //}

  //float deltaSmallHPrime = 0;
  //float bigHBarPrime = 0;
  
  //if (cPrime_1 == 0 || cPrime_2 == 0) {
    //bigHBarPrime = hPrime_1 + hPrime_2;
  //}
  //else {
    //if (fabs(hPrime_2 - hPrime_1) <= 180) {
      //deltaSmallHPrime = hPrime_2 - hPrime_1;
      //bigHBarPrime = (hPrime_1 + hPrime_2) / 2.0f;
    //}
    //else {
      //if (hPrime_2 - hPrime_1 > 180.0f) {
        //deltaSmallHPrime = hPrime_2 - hPrime_1 - 360;
      //}
      //else if (hPrime_2 - hPrime_1 < -180.0f){
        //deltaSmallHPrime = hPrime_2 - hPrime_1 + 360;
      //}
      //bigHBarPrime = (hPrime_1 + hPrime_2 + 360) / 2.0f;
    //}
  //}

  //float deltaBigHPrime = 0;
  //if (cPrime_1 == 0 || cPrime_2 == 0) {  
    //deltaBigHPrime = 2 * sqrt(cPrime_1 * cPrime_2) * sinDeg(deltaSmallHPrime / 2.0f);
  //}

  //float t = 1 - 0.17*cosDeg(bigHBarPrime-30) + 0.24*cosDeg(2*bigHBarPrime) + 
                //0.32*cosDeg(3*bigHBarPrime+6) - 0.20*cosDeg(4*bigHBarPrime - 63); 

  //float S_L = 1.0f + 0.015*pow(lBar-50,2)/sqrt(20+pow(lBar-50,2));
  //float S_C = 1 + 0.045*cBarPrime;
  //float S_H = 1 + 0.015*cBarPrime*t;

  //// 60deg inside sin
  //float R_T = -2*cBarPrimeIntermediary*sin(60.0f * exp(-pow((bigHBarPrime-275.0f)/25.0f,2)));

  //constexpr float k_L = 1;
  //constexpr float k_C = 1;
  //constexpr float k_H = 1;
  
  //return sqrt(pow(deltaLPrime/(k_L*S_L),2) + pow(deltaCPrime/(k_C*S_C),2) + pow(deltaBigHPrime/(k_H*S_H),2) +
         //R_T*deltaCPrime*deltaBigHPrime/(k_C*S_C*k_H*S_H));
  //return pow(deltaLPrime/(k_L*S_L),2) + pow(deltaCPrime/(k_C*S_C),2) + pow(deltaBigHPrime/(k_H*S_H),2) +
         //R_T*deltaCPrime*deltaBigHPrime/(k_C*S_C*k_H*S_H);

  #elif defined(COLDIST_CIE94)
  // CIE1994 implementation
  // first find LCH 
  lchIntermediary pA = {(float)data.l, (float)sqrt(pow(data.a,2) + pow(data.b,2)), 0*(float)atan2(data.b, data.a)};
  lchIntermediary pB = {(float)point.data.l, (float)sqrt(pow(point.data.a,2) + pow(point.data.b,2)), 
                        0*(float)atan2(point.data.b, point.data.a)};

  float deltaL = pA.l - pB.l;
  float deltaC = pA.c - pB.c;
  float deltaH = pow(data.a-point.data.a,2) + pow(data.b-point.data.b, 2) - pow(deltaC, 2);


  // preset (graphics_art) for  k_* determinants

  constexpr float k_L = 1;
  constexpr float k_C = 1;
  constexpr float k_H = 1;

  constexpr float K_1 = 0.045;
  constexpr float K_2 = 0.015;
  
  constexpr float S_L = 1;
  float S_C = 1 + K_1*pA.c;
  float S_H = 1 + K_2*pA.c;  


  //sqrt omitted for efficiency, not when debug
  return sqrt(pow(deltaL/(k_L*S_L),2) + pow(deltaC/(k_C*S_C),2) + deltaH*pow(1/(k_H*S_H),2)); 
  return pow(deltaL/(k_L*S_L),2) + pow(deltaC/(k_C*S_C),2) + deltaH*pow(1/(k_H*S_H),2); 

  #elif defined(COLDIST_CIE76)
  // CIE1976 deltaE
  //sqrt omitted for efficiency, not when debug
  return sqrt(pow(point.data.l - data.l, 2) + pow(point.data.a - data.a, 2) + pow(point.data.b - data.b, 2));
  return pow(point.data.l - data.l, 2) + pow(point.data.a - data.a, 2) + pow(point.data.b - data.b, 2);
  #else
  return 0;
  #endif
}
