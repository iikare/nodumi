#include <functional>
#include <iostream>
#include <random>
#include <vector>
#include <ctime>
#include "color.h"
#include "image.h"
#include "colorgen.h"

using std::mt19937;
using std::uniform_int_distribution;
using std::vector;
using std::cerr;
using std::endl;
using std::bind;

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

  if (n == 1) {
    // handle k = 1 later 
    return;
  }

  if (!k) {
    // 2k distinct colors
    k = n;
  }

  //colorVecA.clear();
  //colorVecB.clear();
  
  int kWidth, kHeight = 0;
  vector<pixel> colorData = image->getKMeansSample(kWidth, kHeight); 
 
  vector<colorRGB> colorVecC = findKMeans(colorData, kWidth, kHeight, k);

  for (unsigned int i = 0; i < colorVecC.size(); i++) {
    cerr << colorVecC[i] << endl;
  }

  //find n shades of k colors

}

vector<colorRGB> findKMeans(vector<pixel>& colorData, int kWidth, int kHeight, int k) {
  vector<colorRGB> result;
  vector<pixel> centroidData;

  mt19937::result_type gen = time(0);
  uniform_int_distribution<int> rng(0, colorData.size() - 1);
  auto getCentroid = bind(rng, mt19937(gen));
  vector<int> nPixel(k, 0);
  vector<double> sumR(k, 0); 
  vector<double> sumG(k, 0); 
  vector<double> sumB(k, 0);

  // generate random starting points
  for (int i = 0; i < k; ++i) {
    cerr << getCentroid() << endl;
    centroidData.push_back(colorData[getCentroid()]);
  }

  // run algorithm q times
  for (int q = 0; q < 12; q++) {
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


