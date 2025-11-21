#pragma once

#include <memory>
#include <vector>

#include "build_target.h"
#include "log.h"

using std::unique_ptr;
using std::vector;

#ifdef USE_TORCH_CLASSIFIER
  #include <torch/script.h>
  #include <torch/torch.h>
#endif

class inputClassifier {
 public:
  void init();
  void terminate();
  int run_inference(vector<int>& seq, int current);
  int get_length() { return sequence_length; }
  bool enabled();

 private:
  static constexpr int sequence_length = 16;
#ifdef USE_TORCH_CLASSIFIER
  torch::DeviceType device_type = torch::kCPU;
  unique_ptr<torch::jit::script::Module> classifier;
#endif
};
