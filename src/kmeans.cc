#include "kmeans.h"

#include "cie2k.h"
#include "define.h"
#include "log.h"
#include "wrap.h"

double kMeansPoint::distance(const kMeansPoint& point) {
  return deltaE(data, point.data);
}
