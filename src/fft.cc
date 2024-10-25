#include "fft.h"

#include "define.h"

using std::swap;

double fftController::getFundamental(int y) { return pow(2, (y - 69) / 12.0) * 440; }

// f_1 is the original frequency, f_2 is the correlation frequency
double fftController::fftAC(double f_1, double f_2) {
  // func: sin(f_1) * sin(f_2)

  double sum = 0;
  // NOTE: SIMD optimizations disabled on debug builds
  OPENMP_USE_SIMD
  for (auto i = 0; i < FFT_AC_BINS; ++i) {
    double fN = sin(f_1) * sin(f_2);
    double fNpK = sin(f_1 + i) * sin(f_2 + i);
    sum += log(1 + fabs(fN * fNpK / ((f_1 - f_2) / f_1)));
  }

  return fabs(sum / FFT_AC_BINS);
}

void fftController::updateFFTBins() {
  crit.lock();
  int nBins = ctr.getWidth() / FFT_BIN_WIDTH - 2;
  double binFreq = log10(static_cast<float>(FFT_MAX_FREQ) / FFT_MIN_FREQ) / (nBins - 1);

  // frequency, height
  bins.clear();
  bins.resize(nBins, make_pair(0.0, 0));

  bin_map.clear();
  bin_map.resize(nBins);
  bin_map_last.clear();
  bin_map_last.resize(nBins);

  for (unsigned int bin = 0; bin < bins.size(); ++bin) {
    bins[bin].first = 20 * pow(10, bin * binFreq);
    bin_map[bin] = vector<pair<int, int>>{};
    bin_map_last[bin] = vector<pair<int, int>>{};
  }
  crit.unlock();
}

void fftController::generateFFTBins(const vector<int> c_note, double offset) {
  generator = thread(&fftController::generate, this, c_note, offset);
}

void fftController::generate(const vector<int> c_note, double offset) {
  crit.lock();
  auto& notes = ctr.getNotes();

  for (unsigned int bin = 0; bin < bins.size(); ++bin) {
    bin_map[bin].clear();
  }

#pragma omp parallel for num_threads(4)
  for (unsigned int bin = 0; bin < bins.size(); ++bin) {
    vector<pair<int, int>> result;
    for (const auto& idx : c_note) {
      double freq = getFundamental(notes[idx].y);
      double nowRatio = (offset - notes[idx].x) / (notes[idx].duration);
      double pitchRatio = 0;
      if (nowRatio > -0.25 && nowRatio < 0) {
        pitchRatio = 16 * pow(nowRatio + 0.25, 2);
      }
      else if (nowRatio < 1) {
        pitchRatio = 1 - 1.8 * pow(nowRatio, 2) / 4.5;
      }
      else if (nowRatio < 1.78) {  // TODO: smoothen interpolation functions
        pitchRatio = 1.0 * pow(nowRatio - 1.78,
                               2);  // TODO: make function duration-invariant
      }
      int binScale = 2 + 5 * (1 + log(1 + notes[idx].duration)) + (15.0 / 128) * ((notes[idx].velocity) + 1);

      // logQ(notes[idx].y, freq, bins.size());
      //  simulate harmonics
      double fftBinLenAll = 0;
      for (unsigned int harmonicScale = 0; harmonicScale < harmonicsSize; ++harmonicScale) {
        double fftBinLen = 0;
        if (freq * harmonics[harmonicScale] < FFT_MIN_FREQ || freq * harmonics[harmonicScale] > FFT_MAX_FREQ) {
          continue;
        }

        fftBinLen += harmonicsCoefficient[harmonicScale] * binScale * pitchRatio *
                     fftAC(freq * harmonics[harmonicScale], bins[bin].first);

        // pseudo-random numerically stable offset
        // TODO: implement spectral rolloff in decay (based on BIN FREQ v.
        // spectral rolloff curve)
        fftBinLen *=
            1 + 0.7 * pow(((ctr.getPSR() ^ static_cast<int>(bins[bin].first)) % static_cast<int>(bins[bin].first)) /
                                  bins[bin].first -
                              0.5,
                          2);

        fftBinLenAll += fftBinLen;
      }

      result.push_back(make_pair(idx, fftBinLenAll));
    }
#pragma omp critical
    bin_map[bin] = result;
  }

  // normalization
  int bin_max = 0;
  int bin_max2 = 0;
  for (auto& bin : bin_map) {
    int bin_sum = 0;
    for (auto& n : bin) {
      bin_sum += n.second;
    }
    bin_max2 = max(bin_max2, bin_sum);
    if (bin_max2 > bin_max) {
      swap(bin_max, bin_max2);
    }
  }

  // logQ("bin_max:", bin_max, bin_max2);

  constexpr double heightRatio = 0.15;
  if (bin_max2 > ctr.getHeight() * heightRatio) {
    double scaleRatio = ctr.getHeight() * heightRatio / bin_max2;
    // logQ(scaleRatio);

    for (auto& bin : bin_map) {
      for (auto& n : bin) {
        n.second *= scaleRatio;
      }
    }
  }
  crit.unlock();
}

vector<vector<pair<int, int>>>& fftController::getFFTBins() {
  generator_join();
  std::swap(bin_map, bin_map_last);
  for (unsigned int bin = 0; bin < bins.size(); ++bin) {
    bins[bin].second = 0;
  }
  return bin_map_last;
}

void fftController::generator_join() {
  if (generator.joinable()) {
    generator.join();
  }
}
