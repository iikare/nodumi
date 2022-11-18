#include <unordered_map>
#include <functional>
#include <algorithm>
#include <random>
#include <vector>
#include <chrono>
#include <thread>
#include <map>
#include "log.h"
#include "color.h"
#include "colorgen.h"
#include "define.h"

using namespace std::chrono;
using std::mt19937;
using std::uniform_int_distribution;
using std::vector;
using std::swap;
using std::bind;
using std::pair;
using std::min;

void invertColorScheme(colorRGB& bg, colorRGB& line, vector<colorRGB>* on, vector<colorRGB>* off){
  bg.invert();
  line.invert();
  for (unsigned int i = 0; i < on->size(); i++) {
    ((*on)[i]).invert();
    ((*off)[i]).invert();
  }
}
void getColorScheme(int n, vector<colorRGB>& colorVecA, vector<colorRGB>& colorVecB) {
  vector<pair<int, double>> tmp;
  getColorScheme(n, colorVecA, colorVecB, tmp);
}

void getColorScheme(int n, vector<colorRGB>& colorVecA, vector<colorRGB>& colorVecB, const vector<pair<int, double>>& weight) {
  
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

  // for bright color
  swap(colorVecA, colorVecB);
}

void getColorSchemeImage(schemeType type, vector<colorRGB>& colorVecA, vector<colorRGB>& colorVecB,
                         const vector<pair<int, double>>& weight) {
  switch (type) {
    case SCHEME_KEY:
      getColorSchemeImage(KEY_COUNT, 2, colorVecA, colorVecB, weight);
      break;
    case SCHEME_TONIC:
      getColorSchemeImage(TONIC_COUNT, 2, colorVecA, colorVecB, weight);
      break;
    case SCHEME_TRACK:
      getColorSchemeImage(ctr.getTrackCount(), 2, colorVecA, colorVecB, weight);
      break;
    default: // nothing here
      break;
  }
}
void getColorSchemeImage(int n, int k, vector<colorRGB>& colorVecA, vector<colorRGB>& colorVecB, 
                         const vector<pair<int, double>>& weight) {

  // check zero k
  if (!n) {
    logW(LL_WARN, "call to getColorSchemeImage with n = ", n);
    return;
  }

  if (!k) {
    // 2k distinct colors
    k = n;
  }

  int nCol = n;
  if (ctr.image.getNumColors() < n) {
    // too many colors from too few unique datapoints
    logW(LL_WARN, "attempt to get", n, "colors from", to_string(ctr.image.getNumColors()) + "-colored image"); 
    nCol = ctr.image.getNumColors();
  }

  vector<kMeansPoint> colorData = ctr.image.getRawData(); 

  // init rng
  mt19937::result_type gen = high_resolution_clock::now().time_since_epoch().count();
  uniform_int_distribution<int> range(0, colorData.size() - 1);
  auto getPoint = bind(range, mt19937(gen));
  
  //return;
  //colorVecA.clear();
  //colorVecB.clear();
  
  //logQ(ctr.getTrackCount());
  vector<colorRGB> colorVecC;

  if (nCol == 1) {
    // single color, pick an edge color
    colorVecC.push_back(colorRGB(colorData[getPoint()].data));
  }
  else if (nCol == 2) {
    //colorVecC.push_back(colorRGB(254,100,255)); 
    //colorVecC.push_back(colorRGB(100,255,255)); 

    vector<pair<int, kMeansPoint>> intermediateSet;

    const auto existsInMap = [&] (const colorLAB col){
      for (const auto& i : intermediateSet) {
        if (&colorData[i.first].data == &col) {
          return true;
        }
      }
      return false; 
    };

    // generate min(100, nCol) unique points
    for (int i = 0; i < min(MAX_UNIQUE_COLORS, nCol); ++i) {
      int intermediateIdx= getPoint();
      colorLAB cenCol = colorData[intermediateIdx].data;
   
      while (existsInMap(cenCol)) { 
        intermediateIdx = getPoint();
        cenCol = colorData[intermediateIdx].data;
      }

      intermediateSet.push_back(make_pair(intermediateIdx, colorData[intermediateIdx]));
    }

    double maxDeltaE = 0;
    int colAIdx = 0;
    int colBIdx = 0;

    for (unsigned int i = 0; i < intermediateSet.size(); ++i) {
      
      for (unsigned int j = i; j < intermediateSet.size(); ++j) {

        double deltaE = intermediateSet[i].second.distance(std::ref(intermediateSet[j].second));

        if (maxDeltaE < deltaE) {
          maxDeltaE = deltaE;
          colAIdx = i;
          colBIdx = j;
        }
      }
    }

    colorVecC.push_back(colorRGB(intermediateSet[colAIdx].second.data));
    colorVecC.push_back(colorRGB(intermediateSet[colBIdx].second.data));
    logQ(intermediateSet[colAIdx].second.data);
    logQ(intermediateSet[colBIdx].second.data);
  
    logQ("max delta E is:", maxDeltaE); 

  

  }
  else {
    colorVecC = findKMeans(colorData, nCol);
    // add any extra values
    if (ctr.image.getNumColors() < n) {
      for (int i = 0; i < n - ctr.image.getNumColors(); ++i) {
        colorVecC.push_back(colorVecC[i % colorVecC.size()]);
      }  
    }
  }
  
  
  //colorVecA = colorVecC;
  //colorVecB = colorVecC;
  
  // k == 1 should never occur

  //if (k == 1) {
    //// filter single color
    //float colV = colorVecC[0].getHSV().v;
    //while (colV < 100) {
      //colorVecC.clear();
      //colorVecC = findKMeans(colorData, k);
      //colV = colorVecC[0].getHSV().v;
    //}
  //}

  // sort if track weight map is provided
  if (weight.size() != 0) {
    for (unsigned int i = 0; i < weight.size(); i++) {
      swap(colorVecA[i], colorVecA[weight[i].first]);
      swap(colorVecB[i], colorVecB[weight[i].first]);
    }
  } 
  
  colorVecA = colorVecC;
  colorVecB = colorVecC;

  constexpr float valScale = 1.2f;

  for (unsigned int i = 0; i < colorVecC.size(); ++i) {
    if (ctr.image.getMeanValue() > colorVecA[i].getHSV().v) {
      colorHSV tmpHSV = colorVecB[i].getHSV();
      tmpHSV.v = min(255.0f, (float)tmpHSV.v*valScale);
      
      colorVecA[i] = tmpHSV.getRGB();
    }
    else {
      colorHSV tmpHSV = colorVecA[i].getHSV();
      tmpHSV.v = max(0.0f, (float)tmpHSV.v*(1.0f/valScale));

      colorVecB[i] = tmpHSV.getRGB();

    }
  }
  return;

  ////find k shades of n colors
  //if (n > k) {
    //if (k != 2) {
      //cerr << "warn: this value of k is not supported for color interpolation (k = " << k << ")" << endl;
      //return;
    //}
    
    ////cerr << colorVecC[0] << endl;
    ////cerr << colorVecC[1] << endl;

    //colorHSV col1(0, 0, 0);
    //colorHSV col2(0 ,0 ,0);
    
    //if (isAOn) {
      //// interpolate using 0->1 values 
      //col1 = colorVecB[0].getHSV();
      //col2 = colorVecB[1].getHSV();
    //}
    //else {
      //// interpolate using 1->0 values
      //col1 = colorVecB[1].getHSV();
      //col2 = colorVecB[0].getHSV();
    //}

    //colorHSV start(0, 0, 0);
    //colorHSV end(0, 0, 0);

    //// find starting values
    //col1.v > col2.v ? start = col2 : start = col1;
    //start == col2 ? end = col1 : end = col2;

    //start.s = min(255.0, end.s * 0.9);
    //start.v = start.v * 0.8;

    //end.s = min(255.0, end.s * 1.6);
    //end.v = min(255.0, end.v * 1.1);

    //if (fabs(start.h - end.h) < 100) {
      //// boost gradient if algorithmic values are too low
      //start.h = fmod(start.h - 10, 360);
      //end.h = fmod(end.h + 10, 360);
    //}
    
    ////cerr << start << endl;
    ////cerr << end << endl;

    //double incH = fabs(start.h - end.h) / n;
    //double incS = fabs(start.s - end.s) / n;
    //double incV = fabs(start.v - end.v) / n;

    ////cerr << "incHSV: " << incH << ", " << incS << ", " << incV << endl; 
  
    //colorVecA.clear();
    //colorVecB.clear();


    //for (int i = 0; i < n; i++) {
      //colorHSV mid = start;
      //mid.h += incH * i;
      //mid.s += incS * i;
      //mid.v += incV * i;
      
      ////cerr << i << ": " << mid << endl;
      
      //colorRGB midRGB;
      //midRGB.setRGB(mid);

      //colorVecB.push_back(midRGB);

      //mid.v = min(255.0, mid.v * 0.8);
      //midRGB.setRGB(mid);

      //colorVecA.push_back(midRGB);
    //}
  //}

  //if (meanV < 200) {
    //// provide contrast on dark images
    //swap(colorVecA, colorVecB);
  //}
  
  //// sort by first 
  //if (weight.size() != 0) {
    //for (unsigned int i = 0; i < weight.size(); i++) {
      //swap(colorVecA[i], colorVecA[weight[i].first]);
      //swap(colorVecB[i], colorVecB[weight[i].first]);
    //}
  //} 

} 

vector<colorRGB> findKMeans(vector<kMeansPoint>& colorData, int k) {

  #if !defined(COLDIST_CIE76) && !defined (COLDIST_CIE94) && !defined(COLDIST_CIE00)
  #error "CIE distance formula not specified at compile time!"
  #endif

  //double maxE = 0;
  //int maxI, maxJ, maxK = 0;
 
  //int r = 0;
  //int g = 255;
  //int b = 0;

  //int _i = 0;
  //int _j = 0;
  //int _k = 0;
  
  //kMeansPoint e(colorRGB(r,g,b));
  //kMeansPoint f(colorRGB(_i,_j,_k));
  
  ////kMeansPoint e(50.0,2.6772,-79.7751);
  ////kMeansPoint f(50.0,0.0,-82.7485);
  //logQ("l a b: ", e.data.l, e.data.a, e.data.b);
  //logQ("l a b: ", f.data.l, f.data.a, f.data.b);

  
  //logQ("deltaE:", e.distance(f), "on map - RGB: {", r, g, b,"} -> RGB: {", _i, _j, _k, "}");

  //for (int i = _i; i<=255; i++) {
    //logQ("i:",i);
    ////for (int j = _j; j<=255; j++) {
      ////for (int k = _k; k<=255; k++) {
        //kMeansPoint f(colorRGB(i,i,i));



        //double q = e.distance(f);
        
        //if (q > maxE) {
          //maxE = q;
          //maxI = i;
          ////maxJ = j;
          ////maxK = k;
        //}
        ////if (q>150)
          ////logQ("rgb:", i, j, k, "deltaE:", q);
      ////}
    ////}
  //}

  //logQ("max deltaE:", maxE, "on map - RGB: {", r, g, b,"} -> RGB: {", maxI, maxI, maxI, "}");
  //exit(1);
  //return vector<colorRGB>();
 
  

  // result container
  vector<colorRGB> colors;
    
  if (!k) {
    logW(LL_WARN, "call to findKMeans with k =", k);
    return colors;
  }



  logQ("");
  logQ("");
  logQ("");
  logQ("");
  logQ("");

  // init rng
  mt19937::result_type gen = high_resolution_clock::now().time_since_epoch().count();
  uniform_int_distribution<int> range(0, colorData.size() - 1);
  auto getCentroid = bind(range, mt19937(gen));
  
  // step1: choose points 
  vector<pair<int, kMeansPoint>> centroids;
  // map of centroids(index in colorData) to points(index in colorData)
  vector<pair<int, vector<int>>> centroidMap;
  //centroidMap.resize(k);

  // inital point
  int centroidIdx = getCentroid();
  colorLAB cenCol = colorData[centroidIdx].data;
  centroids.push_back(make_pair(centroidIdx, colorData[centroidIdx]));
  centroidMap.push_back(make_pair(centroidIdx, vector<int>{}));
 
  const auto existsInMap = [&] (const colorLAB col){
    for (const auto& i : centroids) {
      if (&colorData[i.first].data == &col) {
        return true;
      }
    }
    return false; 
  };

  // transforms a centroid's index in colorData to its index in centroidMap
  //auto getCentroidIndex = [&] (const int idx){
    //for (int i = 0; i < (int)centroidMap.size(); ++i) {
      //if (idx == centroidMap[i].first) {
        //return i;
      //} 
    //}
      //logQ("why is the cluster -1?? for idx:", idx);
    //return 0;
  //};
  
  // choose nCol centroids at random
  for (int i = 0; i < k-1; ++i) { 
    
   
    centroidIdx = getCentroid();
    cenCol = colorData[centroidIdx].data;
    existsInMap(cenCol);
   
    while (existsInMap(cenCol)) { 
      centroidIdx = getCentroid();
      cenCol = colorData[centroidIdx].data;
    } 

    centroids.push_back(make_pair(centroidIdx, colorData[centroidIdx]));
    
    centroidMap.push_back(make_pair(centroidIdx, vector<int>{}));


    //logQ("centroid:",centroidIdx, colorData[centroidIdx].data);
  }

  logQ("there are",centroids.size(), "centroids");

  // the index i here is representative of the ith centroid's data points
  vector<double> sumL(centroids.size(),0.0);;
  vector<double> sumA(centroids.size(),0.0);;
  vector<double> sumB(centroids.size(),0.0);;
  
  // amount of points associated to each centroid
  vector<int> nPoints(centroids.size(),0);


  // iterate over points 
  for (unsigned int it = 0; it < KMEANS_ITERATIONS; ++it) {
    // clear LAB sums
    sumL.clear();
    sumL.resize(centroidMap.size());
    sumA.clear();
    sumA.resize(centroidMap.size());
    sumB.clear();
    sumB.resize(centroidMap.size());

    // clear point counter per centroid
    nPoints.clear();
    nPoints.resize(centroidMap.size());

    // calculate initial distance to centroids and assign centroids to points
    for (unsigned int pixel = 0; pixel < colorData.size(); ++pixel) {
     // first reset distance measurements
      colorData[pixel].cluster = -1;
      colorData[pixel].cDist = __DBL_MAX__;
      for (unsigned int centroid = 0; centroid < centroids.size(); ++centroid) {
        kMeansPoint kmPoint = colorData[pixel];

        sumL.push_back(kmPoint.data.l);
        sumA.push_back(kmPoint.data.a);
        sumB.push_back(kmPoint.data.b);

        // find distance and nearest cluster
        float pixDist = kmPoint.distance(centroids[centroid].second);

        //logQ("pixDist:", pixDist);

        if (pixDist < kmPoint.cDist) {
          colorData[pixel].cDist = pixDist;

          // add INDEX of centroid among other centroids
          colorData[pixel].cluster = centroid;
        }
      }
      // assign this point to the specified cluster vector
      //logQ("cluster, pixel: ", colorData.at(pixel).cluster, pixel);
      centroidMap.at(colorData.at(pixel).cluster).second.push_back(pixel);

      // add this point's LAB values to the sum total for the specified pixel's cluster
      sumL.at(colorData.at(pixel).cluster) += colorData.at(pixel).data.l;
      sumA.at(colorData.at(pixel).cluster) += colorData.at(pixel).data.a;
      sumB.at(colorData.at(pixel).cluster) += colorData.at(pixel).data.b;

      // increment the amount of points assigned to this pixel's centroid
      nPoints.at(colorData.at(pixel).cluster)++;

    }

    // update centroids based on mean of clustered points
    for (unsigned int i = 0; i < centroidMap.size(); ++i) {
      float newL = sumL.at(i) / nPoints.at(i); 
      float newA = sumA.at(i) / nPoints.at(i); 
      float newB = sumB.at(i) / nPoints.at(i);

      centroids.at(i).second.data = colorLAB(newL, newA, newB);
    }
  }


  for (const auto& i : centroidMap) {

    logQ("centroid", i.first, "has", i.second.size(), "linked points with LAB:", colorData[i.first].data);
    colors.push_back(colorRGB(colorData[i.first].data));
  }
  

  return colors;
}
