#include "classifier.h"

#include "data.h"

using std::make_unique;

#ifdef USE_TORCH_CLASSIFIER
  #include <algorithm>
  #include <chrono>
  #include <cstdint>
  #include <cstdlib>
  #include <exception>
  #include <istream>
  #include <numeric>
  #include <streambuf>
  #include <string>

  #include "log.h"
  #include "misc.h"

  // the standard library must be in place before the keyword macros below
  #if __has_include(<bits/stdc++.h>)
    #include <bits/stdc++.h>
  #endif

using std::istream;
using std::streambuf;
using std::string;

// -fno-exceptions rejects libtorch's throw and catch, so here a throw aborts and a catch is dead code
namespace torch_no_exceptions {
[[noreturn]] inline void raise() { std::abort(); }
}  // namespace torch_no_exceptions

  #define try if (true)
  #define catch(...) else if (false) for (const std::exception& e = std::exception(); false;)

  // these throw inside expressions or as throw T(name), where only the comma form parses as intended
  #define throw torch_no_exceptions::raise(),
  #include <c10/util/Registry.h>
  #include <c10/util/TypeIndex.h>
  #if __has_include(<c10/util/string_view.h>)
    #include <c10/util/string_view.h>
  #endif
  #undef throw

  // everywhere else a throw is a statement, and some are a bare rethrow
  #define throw                     \
    if (true)                       \
      torch_no_exceptions::raise(); \
    else
  #include <ATen/ATen.h>
  #include <ATen/Parallel.h>
  #include <c10/core/InferenceMode.h>
  #include <torch/csrc/jit/api/module.h>
  #include <torch/csrc/jit/serialization/import.h>
  #include <torch/cuda.h>
  #undef throw
  #undef catch
  #undef try

namespace {

constexpr int live_threads = 1;  // a single live window is fastest on one thread
constexpr int midi_threads = 2;  // whole files gain from a second thread, more only adds heat
constexpr int midi_batch = 256;
constexpr float pad_onset = 100.0f;  // training padding came from other files, far from the window
constexpr float held_end = 1e6f;     // keys still down have no release yet

struct model_buffer : streambuf {
  // force torch to read model from buffer
  model_buffer(const void* data, size_t size) {
    char* p = const_cast<char*>(static_cast<const char*>(data));
    this->setg(p, p, p + size);
  }

  pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which) override {
    char* new_pos = nullptr;

    if (dir == std::ios_base::beg) {
      new_pos = eback() + off;
    }
    else if (dir == std::ios_base::cur) {
      new_pos = gptr() + off;
    }
    else if (dir == std::ios_base::end) {
      new_pos = egptr() + off;
    }

    if (new_pos < eback() || new_pos > egptr()) {
      return pos_type(-1);
    }

    setg(eback(), new_pos, egptr());
    return pos_type(new_pos - eback());
  }

  pos_type seekpos(pos_type sp, std::ios_base::openmode which) override {
    return seekoff(off_type(sp), std::ios_base::beg, which);
  }
};

// note indices from begin to end in the order the models were trained on: by onset, then pitch
vector<int> sorted_order(const vector<note>& notes, int begin, int end) {
  vector<int> order(end - begin);
  std::iota(order.begin(), order.end(), begin);
  std::stable_sort(order.begin(), order.end(), [&](int a, int b) {
    if (notes[a].x != notes[b].x) {
      return notes[a].x < notes[b].x;
    }
    return notes[a].y < notes[b].y;
  });
  return order;
}

// first index of the run of notes that share the onset of notes[i]
int onset_run_start(const vector<note>& notes, int i) {
  while (i > 0 && notes[i - 1].x == notes[i].x) {
    i--;
  }
  return i;
}

// a batch of windows with the note to classify at index n_past of each row
struct window_batch {
  window_batch(int64_t rows, int64_t width)
      : pitch(at::empty({rows, width}, at::kLong)),
        onset(at::empty({rows, width}, at::kFloat)),
        end(at::empty({rows, width}, at::kFloat)),
        valid(at::empty({rows, width}, at::kBool)),
        lookahead(at::zeros({rows, 1}, at::kFloat)) {}

  // fills row r with the notes around order[pos], oldest first
  void fill(int64_t r, const vector<note>& notes, const vector<int>& order, int pos, int n_past, bool live) {
    const int64_t width = pitch.size(1);
    int64_t* p = pitch.data_ptr<int64_t>() + r * width;
    float* o = onset.data_ptr<float>() + r * width;
    float* e = end.data_ptr<float>() + r * width;
    bool* v = valid.data_ptr<bool>() + r * width;

    // times are relative to the classified note, which keeps float precision in long sessions
    const double origin = notes[order[pos]].x;
    for (int64_t k = 0; k < width; ++k) {
      const int64_t slot = pos - n_past + k;
      v[k] = slot >= 0 && slot < static_cast<int64_t>(order.size());
      if (!v[k]) {
        p[k] = 0;
        o[k] = pad_onset;
        e[k] = pad_onset;
        continue;
      }
      const note& n = notes[order[slot]];
      p[k] = std::clamp(n.y, 0, 127);
      o[k] = (n.x - origin) / UNIT_TPS;
      e[k] = live && n.isOn ? held_end : (n.x + n.duration - origin) / UNIT_TPS;
    }
  }

  at::Tensor pitch;
  at::Tensor onset;
  at::Tensor end;
  at::Tensor valid;
  at::Tensor lookahead;
};

}  // namespace

struct inputClassifier::torch_state {
  struct model {
    torch::jit::Module module;
    bool loaded = false;
    bool live = false;
    int n_past = 0;
    int n_future = 0;

    int width() const { return n_past + 1 + n_future; }
  };

  model& get(CLASSIFIER type) { return models[static_cast<int>(type)]; }

  // runs one batch and returns the more likely hand of each row
  vector<int> run(model& m, window_batch& batch) {
    vector<torch::jit::IValue> inputs = {batch.pitch.to(device), batch.onset.to(device), batch.end.to(device),
                                         batch.valid.to(device)};
    if (m.live) {
      inputs.push_back(batch.lookahead.to(device));
    }
    at::Tensor hands = m.module.forward(inputs).toTensor().argmax(1).to(at::kCPU).contiguous();
    const int64_t* h = hands.data_ptr<int64_t>();
    return vector<int>(h, h + hands.size(0));
  }

  model models[2];
  at::DeviceType device = at::kCPU;
  bool threads_set = false;
};
#else
struct inputClassifier::torch_state {};
#endif

inputClassifier::inputClassifier() : state(make_unique<torch_state>()) {}

inputClassifier::~inputClassifier() = default;

bool inputClassifier::load(CLASSIFIER type, const unsigned char* data, unsigned int size) {
#ifdef USE_TORCH_CLASSIFIER
  const string name = type == CLASSIFIER::LIVE ? "live" : "midi";
  if (data == nullptr || size == 0) {
    logW(LL_WARN, "classifier: no", name, "model in this build");
    return false;
  }

  auto start = std::chrono::high_resolution_clock::now();
  if (!state->threads_set) {
    state->threads_set = true;
    at::set_num_threads(live_threads);
    if (torch::cuda::is_available()) {
      state->device = at::kCUDA;
    }
    logW(LL_INFO, "classifier: inference via", state->device == at::kCUDA ? "CUDA" : "CPU");
  }

  model_buffer buf(data, size);
  istream in(&buf);
  torch::jit::Module module = torch::jit::load(in);

  // the window layout is stored in the model, an older model without it can't be used
  auto& m = state->get(type);
  for (const char* attr : {"n_past", "n_future", "live", "var_lookahead"}) {
    if (!module.hasattr(attr)) {
      logW(LL_WARN, "classifier:", name, "model has no attribute", attr);
      return false;
    }
  }
  const torch::jit::IValue n_past = module.attr("n_past");
  const torch::jit::IValue n_future = module.attr("n_future");
  const torch::jit::IValue live = module.attr("live");
  const torch::jit::IValue var_lookahead = module.attr("var_lookahead");
  if (!n_past.isInt() || !n_future.isInt() || !live.isBool() || !var_lookahead.isBool()) {
    logW(LL_WARN, "classifier:", name, "model has malformed attributes");
    return false;
  }
  if (live.toBool() != (type == CLASSIFIER::LIVE) || live.toBool() != var_lookahead.toBool()) {
    logW(LL_WARN, "classifier:", name, "model was exported for the other input type");
    return false;
  }
  m.n_past = n_past.toInt();
  m.n_future = n_future.toInt();
  m.live = live.toBool();

  // inference optimization
  module.eval();
  m.module = torch::jit::optimize_for_inference(module);
  m.module.to(state->device);
  m.loaded = true;

  // the first calls optimize the graph, so they happen now instead of on the first note
  vector<note> warmup(m.width());
  for (int i = 0; i < m.width(); ++i) {
    warmup[i].x = i * 0.15 * UNIT_TPS;
    warmup[i].duration = 0.3 * UNIT_TPS;
    warmup[i].y = 48 + (i * 7) % 36;
  }
  c10::InferenceMode guard;
  const vector<int> order = sorted_order(warmup, 0, warmup.size());
  for (int rows : {1, 4, 1}) {
    window_batch batch(rows, m.width());
    for (int r = 0; r < rows; ++r) {
      batch.fill(r, warmup, order, m.n_past + r % 2, m.n_past, m.live);
    }
    state->run(m, batch);
  }

  debug_time(start, "classifier " + name);
  logW(LL_INFO, "classifier:", name, "model ready, window", m.n_past, "+ 1 +", m.n_future);
  return true;
#else
  N_UNUSED(type);
  N_UNUSED(data);
  N_UNUSED(size);
  return false;
#endif
}

bool inputClassifier::ready(CLASSIFIER type) const {
#ifdef USE_TORCH_CLASSIFIER
  return state->models[static_cast<int>(type)].loaded;
#else
  N_UNUSED(type);
  return false;
#endif
}

bool inputClassifier::enabled() {
#ifdef USE_TORCH_CLASSIFIER
  return true;
#endif
  return false;
}

void inputClassifier::terminate() {}

vector<int> inputClassifier::classify_live(const vector<note>& notes, const vector<live_query>& queries) {
  vector<int> hands;
#ifdef USE_TORCH_CLASSIFIER
  auto& m = state->get(CLASSIFIER::LIVE);
  if (!m.loaded || queries.empty()) {
    return hands;
  }

  int first = notes.size();
  for (const live_query& q : queries) {
    if (q.index < 0 || q.index >= static_cast<int>(notes.size())) {
      logW(LL_WARN, "classifier: live note", q.index, "out of range");
      return hands;
    }
    first = std::min(first, q.index);
  }

  // live notes arrive in onset order, so only runs of equal onsets need sorting by pitch
  const int begin = onset_run_start(notes, std::max(0, onset_run_start(notes, first) - m.n_past));
  const vector<int> order = sorted_order(notes, begin, notes.size());
  vector<int> position(order.size());
  for (int p = 0; p < static_cast<int>(order.size()); ++p) {
    position[order[p] - begin] = p;
  }

  c10::InferenceMode guard;
  window_batch batch(queries.size(), m.width());
  float* lookahead = batch.lookahead.data_ptr<float>();
  for (int r = 0; r < static_cast<int>(queries.size()); ++r) {
    batch.fill(r, notes, order, position[queries[r].index - begin], m.n_past, true);
    lookahead[r] = std::clamp(queries[r].lookahead, 0.0, max_lookahead);
  }
  hands = state->run(m, batch);
#else
  N_UNUSED(notes);
  N_UNUSED(queries);
#endif
  return hands;
}

vector<int> inputClassifier::classify_file(const vector<note>& notes) {
  vector<int> hands;
#ifdef USE_TORCH_CLASSIFIER
  auto& m = state->get(CLASSIFIER::MIDI);
  if (!m.loaded || notes.empty()) {
    return hands;
  }

  auto start = std::chrono::high_resolution_clock::now();
  c10::InferenceMode guard;
  at::set_num_threads(midi_threads);

  const vector<int> order = sorted_order(notes, 0, notes.size());
  hands.resize(notes.size());
  for (int begin = 0; begin < static_cast<int>(order.size()); begin += midi_batch) {
    const int rows = std::min(midi_batch, static_cast<int>(order.size()) - begin);
    window_batch batch(rows, m.width());
    for (int r = 0; r < rows; ++r) {
      batch.fill(r, notes, order, begin + r, m.n_past, false);
    }
    const vector<int> result = state->run(m, batch);
    for (int r = 0; r < rows; ++r) {
      hands[order[begin + r]] = result[r];
    }
  }

  at::set_num_threads(live_threads);
  debug_time(start, "classifier midi, " + std::to_string(notes.size()) + " notes");
#else
  N_UNUSED(notes);
#endif
  return hands;
}
