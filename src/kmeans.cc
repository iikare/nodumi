#include "kmeans.h"
#include "cie2k.h"
#include "define.h"
#include "wrap.h"
#include "log.h"

double kMeansPoint::distance(const kMeansPoint& point) {




  // data       -> A
  // point.data -> B


  #ifdef COLDIST_CIE00
  
  return deltaE(data, point.data);

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
  //return sqrt(pow(deltaL/(k_L*S_L),2) + pow(deltaC/(k_C*S_C),2) + deltaH*pow(1/(k_H*S_H),2)); 
  return pow(deltaL/(k_L*S_L),2) + pow(deltaC/(k_C*S_C),2) + deltaH*pow(1/(k_H*S_H),2); 

  #elif defined(COLDIST_CIE76)
  // CIE1976 deltaE
  //sqrt omitted for efficiency, not when debug
  //return sqrt(pow(point.data.l - data.l, 2) + pow(point.data.a - data.a, 2) + pow(point.data.b - data.b, 2));
  return pow(point.data.l - data.l, 2) + pow(point.data.a - data.a, 2) + pow(point.data.b - data.b, 2);
  #else
  return 0;
  #endif
}
