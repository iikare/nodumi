#include <functional>
#include <algorithm>
#include <iostream>
#include <random>
#include <vector>
#include <chrono>
#include "color.h"
#include "image.h"
#include "colorgen.h"

using namespace std::chrono;
using std::mt19937;
using std::uniform_int_distribution;
using std::vector;
using std::count;
using std::swap;
using std::cerr;
using std::endl;
using std::bind;
using std::min;

void getColorScheme(int n, vector<colorRGB>& colorVecA, vector<colorRGB>& colorVecB) {

  // check zero n
  if (!n) {
    cerr << "error: call to getColorScheme with n = " << n << endl;
    exit(1);
  }

  // generate color scheme
  colorHSV off (0, 0.7, 115);
  colorHSV on (0, 0.7, 199);

  colorRGB offRGB;
  colorRGB onRGB;

  int increment = 360.0/n;

  colorVecA.clear();
  colorVecB.clear();
  
  for (int i = 0; i < n; i++) {
    off.h = fmod((off.h + increment),360.0);
    on.h = off.h;

    offRGB.setRGB(off);
    onRGB.setRGB(on);
    colorVecA.push_back(offRGB);
    colorVecB.push_back(onRGB);
  }
}

void getColorSchemeBG(BGImage* image, int n, int k, vector<colorRGB>& colorVecA, vector<colorRGB>& colorVecB){
 
  // check zero k
  if (!n) {
    cerr << "error: call to getColorSchemeBG with n = " << n << endl;
    return;
  }

  if (!k) {
    // 2k distinct colors
    k = n;
  }

  colorVecA.clear();
  colorVecB.clear();
  
  vector<pixel> colorData = image->getKMeansSample(); 

  int meanV = 0;
  
  vector<colorRGB> colorVecC = findKMeans(colorData, k, meanV);

  
  if (k == 1) {
    // filter single color
    colorHSV col = colorVecC[0].getHSV();
    while (col.v < 100) {
      colorVecC.clear();
      vector<colorRGB> colorVecC = findKMeans(colorData, k, meanV);
      col.v = colorVecC[0].getHSV().v;
    }
  }

  bool isAOn = false;

  for (unsigned int i = 0; i < colorVecC.size(); i++) {
    colorHSV col(colorVecC[i].getHSV());

    if (col.v > meanV) {
      // this is on color, calculate off
      colorRGB colB;
      colB.setRGB(col);

      col.v = min(255.0, 1.2 * meanV);
      colB.setRGB(col);
      colorVecB.push_back(colB);
      
      col.v = meanV * 0.8;
      colB.setRGB(col);

      colorVecA.push_back(colB);

      isAOn = false;
    }
    else {
      // do inverse
      
      colorRGB colB;
      colB.setRGB(col);

      col.v = meanV * 0.8;
      colB.setRGB(col);
      
      colorVecA.push_back(colB);
      isAOn = true;
      
      col.v = min(255.0, 1.2 * meanV);
      colB.setRGB(col);
      
      colorVecB.push_back(colB);
    }
  }

  //find n shades of k colors
  if (n > k) {
    if (k != 2) {
      cerr << "warn: this value of k is not supported for color interpolation (k = " << k << ")" << endl;
      return;
    }
    
    colorHSV col1(0, 0, 0);
    colorHSV col2(0 ,0 ,0);
    
    if (isAOn) {
      // interpolate using A values 
      col1 = colorVecA[0].getHSV();
      col2 = colorVecA[1].getHSV();
    }
    else {
      // interpolate using B values
      col1 = colorVecB[0].getHSV();
      col2 = colorVecB[1].getHSV();
    }

    double incH = abs(col2.h - col1.h) / n;
    double incS = abs(col2.s - col1.s) / n;
    double incV = abs(col2.v - col1.v) / n;
  
    colorHSV start(0, 0, 0);

    // find starting values
    col1.v > col2.v ? start = col2 : start = col1;

    colorVecA.clear();
    colorVecB.clear();

    for (int i = 0; i < n; i++) {
      colorHSV mid = start;
      start.h += incH * n;
      start.s += incS * n;
      start.v += incV * n;
      
      colorRGB midRGB;
      midRGB.setRGB(mid);

      colorVecB.push_back(midRGB);

      mid.h = min(255.0, mid.h * 1.2);
      midRGB.setRGB(mid);

      colorVecA.push_back(midRGB);
    }
  }

  if (meanV < 200) {
    // provide contrast on dark images
    swap(colorVecA, colorVecB);
  }
  
}

vector<colorRGB> findKMeans(vector<pixel>& colorData, int k, int& meanV) {
  // result vector
  vector<colorRGB> result;

  // mean value
  double intVal = 0;

  // init rng
  mt19937::result_type gen = high_resolution_clock::now().time_since_epoch().count();
  uniform_int_distribution<int> range(0, colorData.size() - 1);
  auto getCentroid = bind(range, mt19937(gen));

  // generate starting points via distance-based selection algorithm
  double maxDist = 0; 
  int idxNextCentroid = 0;
  vector<int> idxUsed(k, 0);
  vector<pixel> centroidData;

  // get mean value if k = 1
  if (k == 1) {
    for (unsigned int i = 0; i < colorData.size(); i++) {
      colorHSV col = colorData[i].data.getHSV();
      intVal += col.v;
    }
    meanV = intVal / colorData.size(); 
  }

  for (int i = 0; i < k; ++i) {
    // initial centroid is random
    if (i == 0) {
      centroidData.push_back(colorData[getCentroid()]);
      if (k == 1) {
        colorRGB col(round(centroidData[i].data.r), round(centroidData[i].data.g), round(centroidData[i].data.b)); 
        result.push_back(col);
        return result;
      }
    }
    else {
      for (unsigned int j = 0; j < colorData.size(); j++) {
        if (i == 1) {
          // only calculate once
          colorHSV col = colorData[j].data.getHSV();
          intVal += col.v;
        }

        double distToCentroid = 0;
        for (int m = 0; m < i; m++) {
          // prevent clustering of centroids
          if (centroidData[m].distance(colorData[j].data) < 100) {
            distToCentroid = 0;
            break;
          }
          distToCentroid += centroidData[m].distance(colorData[j].data);
        }
        if (distToCentroid > maxDist) {
            maxDist = distToCentroid;
            idxNextCentroid = j;
        }  
      }
      idxUsed.push_back(idxNextCentroid);
      centroidData.push_back(colorData[idxNextCentroid]);
      maxDist = 0;
    }
  }

  meanV = intVal / colorData.size(); 

  vector<int> nPixel(k, 0);
  vector<double> sumR(k, 0); 
  vector<double> sumG(k, 0); 
  vector<double> sumB(k, 0);
  
  // run algorithm q times
  for (int q = 0; q < 3; q++) {
    // assign pixels to centroids
    for (unsigned int i = 0; i < centroidData.size(); i++) {
      for (unsigned int j = 0; j < colorData.size(); j++) {
        double dist = colorData[j].distance(centroidData[i].data);
        if (dist < colorData[j].cDist) {
          colorData[j].cDist = dist;
          colorData[j].cluster = i;
        }
      }
    }
    
    // accumulate centroid data
    for (unsigned int i = 0; i < colorData.size(); i++) {
      int nCluster = colorData[i].cluster;

      nPixel[nCluster] += 1;

      sumR[nCluster] += static_cast<double>(colorData[i].data.r);
      sumG[nCluster] += static_cast<double>(colorData[i].data.g);
      sumB[nCluster] += static_cast<double>(colorData[i].data.b);

      colorData[i].cDist = __DBL_MAX__;
    }

    // find new centroids
    for (unsigned int i = 0; i < centroidData.size(); i++) {
      if (nPixel[i] != 0) {
        centroidData[i].data.r = sumR[i]/nPixel[i];
        centroidData[i].data.g = sumG[i]/nPixel[i];
        centroidData[i].data.b = sumB[i]/nPixel[i];
      }
      else {
        // algorithm failed, restart
      }
    }
  }

  // convert to colorRGB vector
  
  for (unsigned int i = 0; i < centroidData.size(); i++) {
    colorRGB col(round(centroidData[i].data.r), round(centroidData[i].data.g), round(centroidData[i].data.b)); 
    result.push_back(col);
  }

  return result;
}


