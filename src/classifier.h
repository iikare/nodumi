#pragma once

#include <memory>
#include <vector>

#include "note.h"

using std::unique_ptr;
using std::vector;

// live input only knows what has been played so far, files are known in full
enum class CLASSIFIER { LIVE, MIDI };

// one live note to classify, decided lookahead seconds after its onset
struct live_query {
  int index;
  double lookahead;
};

class inputClassifier {
 public:
  inputClassifier();
  ~inputClassifier();

  // loads an embedded torchscript model, false when this build or the model can't be used
  bool load(CLASSIFIER type, const unsigned char* data, unsigned int size);
  bool ready(CLASSIFIER type) const;
  void terminate();
  bool enabled();

  // hands (0 = left, 1 = right) of the queried notes, empty when the live model isn't loaded
  vector<int> classify_live(const vector<note>& notes, const vector<live_query>& queries);
  // hands of every note of a prerecorded file, empty when the file model isn't loaded
  vector<int> classify_file(const vector<note>& notes);

  // the live model was trained to wait anywhere from 0 to this many seconds before deciding
  static constexpr double max_lookahead = 0.05;

 private:
  // torch types stay inside classifier.cc, the only file built against libtorch
  struct torch_state;
  unique_ptr<torch_state> state;
};
