#include "kmeans.h"
#include "define.h"
#include "log.h"



double kMeansPoint::distance(kMeansPoint& point) {

  #ifdef COLDIST_CIE00
  // CIEDE2000 implementation

  lchIntermediary pA = {(float)data.l, (float)sqrt(pow(data.a,2) + pow(data.b,2)), 0*(float)atan2(data.b, data.a)};
  lchIntermediary pB = {(float)point.data.l, (float)sqrt(pow(point.data.a,2) + pow(point.data.b,2)), 
                        0*(float)atan2(point.data.b, point.data.a)};
  
  float deltaLPrime = pB.l - pA.l;

  float lBar = (pA.l + pB.l)/2.0f;
  float cBar = (pA.c + pB.c)/2.0f;

  float aPrime_1 = data.a + data.a/2.0f * (1 - sqrt(pow(cBar,7)/(pow(cBar,7)+pow(25,7)))); 
  float aPrime_2 = point.data.a + point.data.a/2.0f * (1 - sqrt(pow(cBar,7)/(pow(cBar,7)+pow(25,7)))); 

  float cPrime_1 = sqrt(pow(aPrime_1, 2) + pow(data.b, 2));
  float cPrime_2 = sqrt(pow(aPrime_2, 2) + pow(point.data.b, 2));

  float deltaCPrime = cPrime_2 - cPrime_1;
  float cBarPrime = (cPrime_1 + cPrime_2) / 2.0f;
  
  float hPrime_1 = fmod(atan2f(data.b, aPrime_1) * 180.0 / M_PI + 360.0f, 360.0f);
  float hPrime_2 = fmod(atan2f(point.data.b, aPrime_2) * 180.0 / M_PI + 360.0f, 360.0f);

  float deltaSmallHPrime = 0;
  float bigHBar = 0;

  if (fabs(hPrime_1 - hPrime_2) <= 180) {
    deltaSmallHPrime = hPrime_2 - hPrime_1;
    bigHBar = (hPrime_1 + hPrime_2) / 2.0f;
  }
  else {
    if (hPrime_1 >= hPrime_2) {
      deltaSmallHPrime = hPrime_2 - hPrime_1 + 360;
    }
    else {
      deltaSmallHPrime = hPrime_2 - hPrime_1 - 360;
    }
    bigHBar = (hPrime_1 + hPrime_2 + 360) / 2.0f;
  }

  float deltaBigHPrime = 2 * sqrt(cPrime_1 * cPrime_2) * 180.0 / M_PI * sin(deltaSmallHPrime / 2.0f);

  #define sinDeg(x) sin(x)* 180.0/M_PI

  float t = 1 - 0.17*sinDeg(bigHBar-30) + 0.24*sinDeg(2*bigHBar) + 0.32*sinDeg(3*bigHBar+6) - 0.20*sinDeg(4*bigHBar - 63); 

  float S_L = 1.0f + 0.015*pow(lBar-50,2)/sqrt(20+pow(lBar-50,2));
  float S_C = 1 + 0.045*cBarPrime;
  float S_H = 1 + 0.015*cBarPrime*t;

  // 60deg inside sin
  float R_T = -2*sqrt(pow(cBarPrime,7)/(pow(cBarPrime,7)+pow(25,7))) * sinDeg(M_PI/3.0f * exp(-pow((bigHBar-275.0f)/25.0f,2)));

  constexpr float k_L = 1;
  constexpr float k_C = 1;
  constexpr float k_H = 1;
  
  return sqrt(pow(deltaLPrime/(k_L*S_L),2) + pow(deltaCPrime/(k_C*S_C),2) + pow(deltaBigHPrime/(k_H*S_H),2) +
         R_T*deltaCPrime*deltaBigHPrime/(k_C*S_C*k_H*S_H));
  return pow(deltaLPrime/(k_L*S_L),2) + pow(deltaCPrime/(k_C*S_C),2) + pow(deltaBigHPrime/(k_H*S_H),2) +
         R_T*deltaCPrime*deltaBigHPrime/(k_C*S_C*k_H*S_H);

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
