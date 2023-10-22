#include "kmeans.h"
#include "cie2k.h"
#include "define.h"
#include "wrap.h"
#include "log.h"

double kMeansPoint::distance(const kMeansPoint& point) {
  return deltaE(data, point.data);
}
