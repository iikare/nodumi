#include "colorgen.h"

#include <algorithm>
#include <chrono>
#include <functional>
#include <map>
#include <random>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "color.h"
#include "data.h"
#include "define.h"
#include "lerp.h"
#include "log.h"

using namespace std::chrono;
using std::bind;
using std::min;
using std::mt19937;
using std::pair;
using std::swap;
using std::uniform_int_distribution;
using std::vector;

void invertColorScheme(colorRGB& bg, colorRGB& line, vector<colorRGB>* on,
                       vector<colorRGB>* off) {
  bg.invert();
  line.invert();
  for (unsigned int i = 0; i < on->size(); i++) {
    ((*on)[i]).invert();
    ((*off)[i]).invert();
  }
}

void getColorScheme(int n, vector<colorRGB>& colorVecA,
                    vector<colorRGB>& colorVecB,
                    const vector<pair<int, double>>& weight) {
  // check zero n
  if (!n) {
    logW(LL_WARN, "call to getColorScheme with n", n);
  }

  // generate color scheme
  colorHSV off(0, 0.7, 115);
  colorHSV on(0, 0.7, 199);

  colorRGB offRGB;
  colorRGB onRGB;

  int increment = 360.0 / n;

  colorVecA.resize(n);
  colorVecB.resize(n);
  colorVecA.clear();
  colorVecB.clear();

  for (int i = 0; i < n; i++) {
    off.h = fmod((off.h + increment), 360.0);
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

void getColorSchemeImage(schemeType type, vector<colorRGB>& colorVecA,
                         vector<colorRGB>& colorVecB,
                         const vector<pair<int, double>>& weight) {
  switch (type) {
    case SCHEME_KEY:
      getColorSchemeImage(2, 2, colorVecA, colorVecB, weight);

      colorVecA.resize(KEY_COUNT);
      colorVecB.resize(KEY_COUNT);

      colorVecA[KEY_COUNT - 1] = colorVecA[1];
      colorVecB[KEY_COUNT - 1] = colorVecB[1];

      for (unsigned int i = 1; i < KEY_COUNT - 1; ++i) {
        double ratio = static_cast<double>(i) / (KEY_COUNT - 1);
        colorVecA[i] = colorLERP(colorVecA[0], colorVecA[KEY_COUNT - 1], ratio);
        colorVecB[i] = colorLERP(colorVecB[0], colorVecB[KEY_COUNT - 1], ratio);
      }
      break;
    case SCHEME_TONIC:
      getColorSchemeImage(TONIC_COUNT, 2, colorVecA, colorVecB, weight);
      break;
    case SCHEME_TRACK:
      getColorSchemeImage(ctr.getTrackCount(), 2, colorVecA, colorVecB, weight);
      break;
    default:  // nothing here
      break;
  }
}

void getColorSchemeImage(int n, int k, vector<colorRGB>& colorVecA,
                         vector<colorRGB>& colorVecB,
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
    logW(LL_WARN, "attempt to get", n, "colors from",
         to_string(ctr.image.getNumColors()) + "-colored image");
    nCol = ctr.image.getNumColors();
  }

  vector<kMeansPoint> colorData = ctr.image.getRawData();

  // init rng
  mt19937::result_type gen =
      high_resolution_clock::now().time_since_epoch().count();
  uniform_int_distribution<int> range(0, colorData.size() - 1);
  auto getPoint = bind(range, mt19937(gen));

  // return;
  // colorVecA.clear();
  // colorVecB.clear();

  // logQ(ctr.getTrackCount());
  vector<colorRGB> colorVecC;

  if (nCol == 1) {
    // single color, pick an edge color
    colorVecC.push_back(colorRGB(colorData[getPoint()].data));
  }
  else if (nCol == 2) {
    vector<pair<int, kMeansPoint>> intermediateSet;
    const auto existsInMap = [&](const colorLAB& col) {
      for (const auto& i : intermediateSet) {
        if (colorData[i.first].data == col) {
          return true;
        }
      }
      return false;
    };

    // sample a small amount of colors
    constexpr int sampleLimit = 4;
    for (int i = 0;
         i < min(MAX_UNIQUE_COLORS, min(ctr.image.getNumColors(), sampleLimit));
         ++i) {
      int intermediate_idx = getPoint();
      colorLAB cenCol = colorData[intermediate_idx].data;

      while (existsInMap(cenCol)) {
        intermediate_idx = getPoint();
        cenCol = colorData[intermediate_idx].data;
      }

      intermediateSet.push_back(
          make_pair(intermediate_idx, colorData[intermediate_idx]));
    }

    double maxDeltaE = 0;
    int colAIdx = 0;
    int colBIdx = 0;

    for (unsigned int i = 0; i < intermediateSet.size(); ++i) {
      for (unsigned int j = i; j < intermediateSet.size(); ++j) {
        double deltaE = intermediateSet[i].second.distance(
            std::ref(intermediateSet[j].second));
        if (maxDeltaE < deltaE) {
          maxDeltaE = deltaE;
          colAIdx = i;
          colBIdx = j;
        }
      }
    }

    colorVecC.push_back(colorRGB(intermediateSet[colAIdx].second.data));
    colorVecC.push_back(colorRGB(intermediateSet[colBIdx].second.data));
    // logQ(intermediateSet[colAIdx].second.data);
    // logQ(intermediateSet[colBIdx].second.data);
    // logQ("max delta E is:", maxDeltaE);
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

  colorVecA = colorVecC;
  colorVecB = colorVecC;
  constexpr float valScale = 1.3f;

  for (unsigned int i = 0; i < colorVecC.size(); ++i) {
    if (ctr.image.getMeanValue() > colorVecA[i].getHSV().v) {
      colorHSV tmpHSV = colorVecB[i].getHSV();
      tmpHSV.v = min(255.0f, (float)tmpHSV.v * valScale);
      tmpHSV.s = min(1.0, tmpHSV.s * valScale);
      colorVecA[i] = tmpHSV.getRGB();
    }
    else {
      colorHSV tmpHSV = colorVecA[i].getHSV();
      tmpHSV.v = max(0.0f, (float)tmpHSV.v * (1.0f / valScale));
      tmpHSV.s = max(0.0, tmpHSV.s * (1.0f / valScale));
      colorVecB[i] = tmpHSV.getRGB();
    }
  }

  // sort color sets by hue
  vector<pair<int, colorHSV>> setHSV(colorVecA.size());
  for (unsigned int i = 0; i < colorVecA.size(); ++i) {
    setHSV[i] = make_pair(i, colorVecA[i].getHSV());
  }

  sort(setHSV.begin(), setHSV.end(),
       [&](const auto a, const auto b) { return a.second.h < b.second.h; });

  for (unsigned int i = 0; i < colorVecA.size(); i++) {
    swap(colorVecA[i], colorVecA[setHSV[i].first]);
    swap(colorVecB[i], colorVecB[setHSV[i].first]);
  }

  // sort if track weight map is provided
  if (weight.size() != 0) {
    for (unsigned int i = 0; i < weight.size(); i++) {
      swap(colorVecA[i], colorVecA[weight[i].first]);
      swap(colorVecB[i], colorVecB[weight[i].first]);
    }
  }
}

vector<colorRGB> findKMeans(vector<kMeansPoint>& colorData, int k) {
  // auto start = std::chrono::high_resolution_clock::now();
  //  result container
  vector<colorRGB> colors(k);

  if (!k) {
    logW(LL_WARN, "call to findKMeans with k =", k);
    return colors;
  }

  // logE();

  // init rng
  mt19937::result_type gen =
      high_resolution_clock::now().time_since_epoch().count();
  uniform_int_distribution<int> range(0, colorData.size() - 1);
  auto getCentroid = bind(range, mt19937(gen));

  // choose points
  vector<pair<int, kMeansPoint>> centroids;
  int centroidIdx = getCentroid();
  colorLAB cenCol = colorData[centroidIdx].data;
  centroids.push_back(make_pair(centroidIdx, colorData[centroidIdx]));

  const auto existsInMap = [&](const colorLAB col) {
    for (const auto& i : centroids) {
      if (colorData[i.first].data == col) {
        return true;
      }
    }
    return false;
  };

  for (int i = 0; i < k - 1; ++i) {
    centroidIdx = getCentroid();
    cenCol = colorData[centroidIdx].data;
    existsInMap(cenCol);

    while (existsInMap(cenCol)) {
      centroidIdx = getCentroid();
      cenCol = colorData[centroidIdx].data;
    }

    centroids.push_back(make_pair(centroidIdx, colorData[centroidIdx]));
    // logQ("centroid:",centroidIdx, colorData[centroidIdx].data);
  }

  // logQ("there are",centroids.size(), "centroids");
  vector<int> nPoints(centroids.size(), 0);

  // iterate over points
  for (unsigned int it = 0; it < KMEANS_ITERATIONS; ++it) {
    nPoints.clear();

#pragma omp parallel for
    for (unsigned int pixel = 0; pixel < colorData.size(); ++pixel) {
      colorData[pixel].cluster = -1;
      colorData[pixel].cDist = __DBL_MAX__;
      for (unsigned int centroid = 0; centroid < centroids.size(); ++centroid) {
        kMeansPoint& kmPoint = colorData[pixel];
        // find distance and nearest cluster
        float pixDist = kmPoint.distance(centroids[centroid].second);

        if (pixDist < kmPoint.cDist) {
          kmPoint.cDist = pixDist;
          // add INDEX of centroid among other centroids
          kmPoint.cluster = centroid;
        }
      }
    }

    vector<colorLAB> centroidSum(centroids.size(), {0.0, 0.0, 0.0});

#pragma omp parallel for
    for (unsigned int centroid = 0; centroid < centroids.size(); ++centroid) {
      for (unsigned int pixel = 0; pixel < colorData.size(); ++pixel) {
        if (static_cast<int>(centroid) == colorData[pixel].cluster) {
          centroidSum[centroid].l += colorData[pixel].data.l;
          centroidSum[centroid].a += colorData[pixel].data.a;
          centroidSum[centroid].b += colorData[pixel].data.b;
          nPoints[centroid]++;
        }
      }
    }

    for (unsigned int centroid = 0; centroid < centroids.size(); ++centroid) {
      float newL = centroidSum[centroid].l / nPoints[centroid];
      float newA = centroidSum[centroid].a / nPoints[centroid];
      float newB = centroidSum[centroid].b / nPoints[centroid];
      centroids[centroid].second.data = colorLAB(newL, newA, newB);
    }
  }

  for (unsigned int centroid = 0; centroid < centroids.size(); ++centroid) {
    // logQ("centroid", centroids[centroid].first, "has", nPoints[centroid],
    //"linked points with LAB:", colorData[centroids[centroid].first].data);
    colors[centroid] = (colorRGB(centroids[centroid].second.data));
  }

  // debug_time(start);

  return colors;
}
