#include "classifier.h"

#include <chrono>
#include <string>

#include "controller.h"
#include "define.h"

using std::istream;
using std::make_unique;
using std::streambuf;
using std::string;

#ifdef USE_TORCH_CLASSIFIER
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
#endif

void inputClassifier::init() {
#ifdef USE_TORCH_CLASSIFIER
  auto start = std::chrono::high_resolution_clock::now();
  classifier = make_unique<torch::jit::script::Module>();

  auto model_data = ctr.getModel("INPUT_CLASS");
  model_buffer buf(model_data.first, model_data.second);
  std::istream in(&buf);
  *classifier = torch::jit::load(in);

  // inference optimization
  classifier->eval();
  *classifier = torch::jit::optimize_for_inference((*classifier));

  // find and store target device
  string device_str = "CPU";
  device_type = torch::kCPU;
  if (torch::cuda::is_available()) {
    device_type = torch::kCUDA;
    device_str = "CUDA";
  }
  logW(LL_INFO, "classifier: inference via", device_str);
  classifier->to(device_type);

  // create dummy inputs for warmup
  auto seq_track = torch::randint(0, 2, {1, sequence_length}, torch::kLong);
  auto seq_y = torch::randint(0, 128, {1, sequence_length}, torch::kLong);
  auto stacked = torch::stack({seq_y, seq_track}, 2);
  auto x_seq = stacked.view({1, sequence_length * 2}).to(device_type);
  auto x_next = torch::randint(0, 128, {1}, torch::kLong).to(device_type);

  vector<torch::jit::IValue> inputs;
  inputs.push_back(x_seq);
  inputs.push_back(x_next);

  // warmup model
  logW(LL_INFO, "classifier: warmup");
  for (int i = 0; i < 10; ++i) {
    classifier->forward(inputs);
  }

  if (device_type == torch::kCUDA) {
    torch::cuda::synchronize();
  }

  debug_time(start, "classifier");
  logW(LL_INFO, "classifier: init complete!");
#endif
}

bool inputClassifier::enabled() {
#ifdef USE_TORCH_CLASSIFIER
  return true;
#endif
  return false;
}

void inputClassifier::terminate() {}

int inputClassifier::run_inference(vector<int>& seq, int current) {
#ifdef USE_TORCH_CLASSIFIER
  if (seq.size() != 2 * sequence_length) {
    logW(LL_WARN, "invalid LSTM sequence length", seq.size(), "- expecting", 2 * sequence_length);
    return 0;
  }

  // convert inputs to tensor
  auto options = torch::TensorOptions().dtype(torch::kInt);
  torch::Tensor x_seq = torch::from_blob(seq.data(), {1, (long)seq.size()}, options);
  auto x_next = torch::tensor({current}, torch::kLong).to(device_type);

  std::vector<torch::jit::IValue> inputs;
  inputs.push_back(x_seq);
  inputs.push_back(x_next);

  auto start = std::chrono::high_resolution_clock::now();

  // begin inference
  auto output = classifier->forward(inputs).toTensor();
  // wait on inference completion (on GPU)
  if (device_type == torch::kCUDA) {
    torch::cuda::synchronize();
  }

  auto end = std::chrono::high_resolution_clock::now();

  debug_time(start, "inference");

  // extract result
  output = output.to(torch::kCPU);
  torch::Tensor prediction = std::get<1>(torch::max(output, 1));

  return prediction.item<long>();
#else
  N_UNUSED(seq);
  N_UNUSED(current);
  return 0;
#endif
}
