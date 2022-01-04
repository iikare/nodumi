#include <functional>
#include <algorithm>
#include <iostream>
#include <random>
#include <vector>
#include <chrono>
#include <thread>
#include "log.h"
#include "color.h"
#include "colorgen.h"
#include "define.h"

using namespace std::chrono;
using std::mt19937;
using std::uniform_int_distribution;
using std::vector;
using std::vector;
using std::swap;
using std::bind;
using std::pair;
using std::min;

using std::thread;

void invertColorScheme(colorRGB& bg, colorRGB& line, vector<colorRGB>* on, vector<colorRGB>* off){
  bg.invert();
  line.invert();
  for (unsigned int i = 0; i < on->size(); i++) {
    (on->at(i)).invert();
    (off->at(i)).invert();
  }
}
void getColorScheme(int n, vector<colorRGB>& colorVecA, vector<colorRGB>& colorVecB) {
  vector<pair<int, double>> tmp;
  getColorScheme(n, colorVecA, colorVecB, tmp);
}

void getColorScheme(int n, vector<colorRGB>& colorVecA, vector<colorRGB>& colorVecB, vector<pair<int, double>>& weight) {
  
  // check zero n
  if (!n) {
    logW(LL_WARN, "call to getColorScheme with n", n);
  }

  // generate color scheme
  colorHSV off (0, 0.7, 115);
  colorHSV on (0, 0.7, 199);

  colorRGB offRGB;
  colorRGB onRGB;

  int increment = 360.0/n;

    colorVecA.resize(n);
    colorVecB.resize(n);
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

  if (weight.size() != 0) {
    for (unsigned int i = 0; i < weight.size(); i++) {
      swap(colorVecA[i], colorVecA[weight[i].first]);
      swap(colorVecB[i], colorVecB[weight[i].first]);
    }
  } 
}

void getColorSchemeImage(int n, vector<colorRGB>& colorVecA, vector<colorRGB>& colorVecB) {
  vector<pair<int, double>> tmp;
  getColorSchemeImage(n, 2, colorVecA, colorVecB, tmp);
}

void getColorSchemeImage(int n, vector<colorRGB>& colorVecA, vector<colorRGB>& colorVecB,
                         vector<pair<int, double>>& weight) {
  getColorSchemeImage(n, 2, colorVecA, colorVecB, weight);
}
void getColorSchemeImage(int n, int k, vector<colorRGB>& colorVecA, vector<colorRGB>& colorVecB, 
                         vector<pair<int, double>>& weight) {
 
  // check zero k
  if (!n) {
    logW(LL_WARN, "error: call to getColorSchemeImage with n = ", n);
    return;
  }

  if (!k) {
    // 2k distinct colors
    k = n;
  }

  vector<kMeansPoint> colorData = ctr.image.getRawData(); 

  colorVecA.clear();
  colorVecB.clear();
  
  //logQ(ctr.getTrackCount());
  int meanV = 0;
  vector<colorRGB> colorVecC = findKMeans(colorData, k, meanV);

  bool isAOn = false;
  
  if (k == 1) {
    // filter single color
    colorHSV col = colorVecC[0].getHSV();
    while (col.v < 100) {
      colorVecC.clear();
      colorVecC = findKMeans(colorData, k, meanV);
      col.v = colorVecC[0].getHSV().v;
    }
  }

  for (unsigned int i = 0; i < colorVecC.size(); i++) {
    colorHSV col(colorVecC[i].getHSV());

    if (col.v > meanV) {
      // this is on color, calculate off
      colorRGB colB;
      colB.setRGB(col);

      col.v = min(255.0, 1.6 * meanV);
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
      
      col.v = min(255.0, 1.6 * meanV);
      colB.setRGB(col);
      
      colorVecB.push_back(colB);
    }
  }

  //find k shades of n colors
  if (n > k) {
    if (k != 2) {
      cerr << "warn: this value of k is not supported for color interpolation (k = " << k << ")" << endl;
      return;
    }
    
    //cerr << colorVecC[0] << endl;
    //cerr << colorVecC[1] << endl;

    colorHSV col1(0, 0, 0);
    colorHSV col2(0 ,0 ,0);
    
    if (isAOn) {
      // interpolate using 0->1 values 
      col1 = colorVecB[0].getHSV();
      col2 = colorVecB[1].getHSV();
    }
    else {
      // interpolate using 1->0 values
      col1 = colorVecB[1].getHSV();
      col2 = colorVecB[0].getHSV();
    }

    colorHSV start(0, 0, 0);
    colorHSV end(0, 0, 0);

    // find starting values
    col1.v > col2.v ? start = col2 : start = col1;
    start == col2 ? end = col1 : end = col2;

    start.s = min(255.0, end.s * 0.9);
    start.v = start.v * 0.8;

    end.s = min(255.0, end.s * 1.6);
    end.v = min(255.0, end.v * 1.1);

    if (fabs(start.h - end.h) < 100) {
      // boost gradient if algorithmic values are too low
      start.h = fmod(start.h - 10, 360);
      end.h = fmod(end.h + 10, 360);
    }
    
    //cerr << start << endl;
    //cerr << end << endl;

    double incH = fabs(start.h - end.h) / n;
    double incS = fabs(start.s - end.s) / n;
    double incV = fabs(start.v - end.v) / n;

    //cerr << "incHSV: " << incH << ", " << incS << ", " << incV << endl; 
  
    colorVecA.clear();
    colorVecB.clear();


    for (int i = 0; i < n; i++) {
      colorHSV mid = start;
      mid.h += incH * i;
      mid.s += incS * i;
      mid.v += incV * i;
      
      //cerr << i << ": " << mid << endl;
      
      colorRGB midRGB;
      midRGB.setRGB(mid);

      colorVecB.push_back(midRGB);

      mid.v = min(255.0, mid.v * 0.8);
      midRGB.setRGB(mid);

      colorVecA.push_back(midRGB);
    }
  }

  if (meanV < 200) {
    // provide contrast on dark images
    swap(colorVecA, colorVecB);
  }
  
  // sort by first 
  if (weight.size() != 0) {
    for (unsigned int i = 0; i < weight.size(); i++) {
      swap(colorVecA[i], colorVecA[weight[i].first]);
      swap(colorVecB[i], colorVecB[weight[i].first]);
    }
  } 

} 

vector<colorRGB> findKMeans(vector<kMeansPoint>& colorData, int k, int& meanV) {

  #if !defined(COLDIST_CIE76) && !defined (COLDIST_CIE94) && !defined(COLDIST_CIE00)
  #error "CIE distance formula not specified at compile time!"
  #endif

  double maxE = 0;
  int maxI, maxJ, maxK = 0;
 
  int r = 255;
  int g = 255;
  int b = 0;

  int _i = 0;
  int _j = 0;
  int _k = 0;
  
  kMeansPoint e(colorRGB(r,g,b));
  kMeansPoint f(colorRGB(_i,_j,_k));
  
  //kMeansPoint e(50.0,2.6772,-79.7751);
  //kMeansPoint f(50.0,0.0,-82.7485);
  logQ("l a b: ", e.data.l, e.data.a, e.data.b);
  logQ("l a b: ", f.data.l, f.data.a, f.data.b);

  
  logQ("deltaE:", e.distance(f), "on map - RGB: {", r, g, b,"} -> RGB: {", _i, _j, _k, "}");

  for (int i = _i; i<=255; i++) {
    logQ("i:",i);
    for (int j = _j; j<=255; j++) {
      for (int k = _k; k<=255; k++) {
        kMeansPoint f(colorRGB(i,j,k));



        double q = e.distance(f);
        
        if (q > maxE) {
          maxE = q;
          maxI = i;
          maxJ = j;
          maxK = k;
        }
        //if (q>150)
          //logQ("rgb:", i, j, k, "deltaE:", q);
      }
    }
  }

  logQ("max deltaE:", maxE, "on map - RGB: {", r, g, b,"} -> RGB: {", maxI, maxJ, maxK, "}");
  exit(1);
  return vector<colorRGB>();
}
  //// result vector
  //vector<colorRGB> result;
  
  //// mean value
  //double intVal = 0;

  //// init rng
  //mt19937::result_type gen = high_resolution_clock::now().time_since_epoch().count();
  //uniform_int_distribution<int> range(0, colorData.size() - 1);
  //auto getCentroid = bind(range, mt19937(gen));

  //// generate starting points via distance-based selection algorithm
  //double maxDist = 0; 
  //int idxNextCentroid = 0;
  //vector<int> idxUsed(k, 0);
  //vector<kMeansPoint> centroidData;

  //if (!k) {
    //logQ(LL_WARN, "call to findKMeans with k =", k);
  //}

  //// get mean value if k = 1
  //if (k == 1) {
    //for (unsigned int i = 0; i < colorData.size(); i++) {
      //colorHSV col = colorData[i].data.getHSV();
      //intVal += col.v;
    //}
    //meanV = intVal / colorData.size(); 
  //}

  //for (int i = 0; i < k; ++i) {
    //// initial centroid is random
    //if (i == 0) {
      //centroidData.push_back(colorData[getCentroid()]);
      //if (k == 1) {
        //colorRGB col(round(centroidData[i].data.r), round(centroidData[i].data.g), round(centroidData[i].data.b)); 
        //result.push_back(col);
        //return result;
      //}
    //}
    //else {
      //for (unsigned int j = 0; j < colorData.size(); j++) {
        //if (i == 1) {
          //// only calculate once
          //colorHSV col = colorData[j].data.getHSV();
          //intVal += col.v;
        //}

        //double distToCentroid = 0;
        //for (int m = 0; m < i; m++) {
          //// prevent clustering of centroids
          

          //// TODO: distance function should return distance not based on raw RGB values but based on distance on 
          //// color perception
          //if (centroidData[m].distance(colorData[j].data) < 10000) {
            //distToCentroid = 0;
            //break;
          //}
          //distToCentroid += centroidData[m].distance(colorData[j].data);
        //}
        //if (distToCentroid > maxDist) {
            //maxDist = distToCentroid;
            //idxNextCentroid = j;
        //}  
      //}
      //idxUsed.push_back(idxNextCentroid);
      //centroidData.push_back(colorData[idxNextCentroid]);
      //maxDist = 0;
    //}
  //}

  //meanV = intVal / colorData.size(); 

  //vector<int> nPixel(k, 0);
  //vector<double> sumR(k, 0); 
  //vector<double> sumG(k, 0); 
  //vector<double> sumB(k, 0);

  //// run algorithm q times
  //for (int q = 0; q < 3; q++) {
    //nPixel.clear();
    //sumR.clear();
    //sumG.clear();
    //sumB.clear();

    //// assign pixels to centroids
    //for (unsigned int i = 0; i < centroidData.size(); i++) {
      //for (unsigned int j = 0; j < colorData.size(); j++) {
        //double dist = colorData[j].distance(centroidData[i].data);
        //if (dist < colorData[j].cDist) {
          //colorData[j].cDist = dist;
          //colorData[j].cluster = i;
        //}
      //}
    //}
    
    //// accumulate centroid data
    //for (unsigned int i = 0; i < colorData.size(); i++) {
      //int nCluster = colorData[i].cluster;

      //nPixel[nCluster] += 1;

      //sumR[nCluster] += static_cast<double>(colorData[i].data.r);
      //sumG[nCluster] += static_cast<double>(colorData[i].data.g);
      //sumB[nCluster] += static_cast<double>(colorData[i].data.b);

      //colorData[i].cDist = __DBL_MAX__;
    //}

    //// find new centroids
    //for (unsigned int i = 0; i < centroidData.size(); i++) {
      //if (nPixel[i] != 0) {
        //centroidData[i].data.r = sumR[i]/nPixel[i];
        //centroidData[i].data.g = sumG[i]/nPixel[i];
        //centroidData[i].data.b = sumB[i]/nPixel[i];
      //}
      //else {
        //// algorithm failed, restart
      //}
    //}
  //}

  //// convert to colorRGB vector
  
  //for (unsigned int i = 0; i < centroidData.size(); i++) {
    //colorRGB col(round(centroidData[i].data.r), round(centroidData[i].data.g), round(centroidData[i].data.b)); 
    //result.push_back(col);
  //}

  //return result;

//}
