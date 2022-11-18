#include "fft.h"
#include "define.h"

double fftController::getFundamental(int y) {
  return pow(2, (y-69)/12.0) * 440;
}

// f_1 is the original frequency, f_2 is the correlation frequency
double fftController::fftAC(double f_1, double f_2) {
  // func: sin(f_1) * sin(f_2)
  
  double sum = 0;
  for (auto i = 0; i < FFT_AC_BINS; ++i) {
    double fN = sin(f_1) * sin(f_2);
    double fNpK = sin(f_1+i) * sin(f_2+i);
    sum+=log(1+fabs(fN*fNpK / ((f_1-f_2)/f_1)));
  }

  return fabs(sum/FFT_AC_BINS);
}

void fftController::updateFFTBins() {
  int nBins = ctr.getWidth()/FFT_BIN_WIDTH - 2;
  double binFreq = log10(static_cast<float>(FFT_MAX_FREQ)/FFT_MIN_FREQ)/(nBins-1);

  // frequency, height
  fftbins.clear();
  fftbins.resize(nBins, make_pair(0.0,0));

  for (unsigned int bin = 0; bin < fftbins.size(); ++bin) {
    fftbins[bin].first = 20*pow(10, bin*binFreq);
  }
}

