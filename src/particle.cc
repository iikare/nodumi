#include "particle.h"
#include "log.h"

using std::remove;
using std::remove_if;
using std::set_difference;

void particleController::update() {
  current_emit_last = current_emit;
  current_emit.clear();
}
    
void particleController::add_emitter(int index) {
  current_emit.push_back(index);
}
    
void particleController::end_emission() {
  current_emit.clear();
  process();
}
    
void particleController::process() {
  vector<int> begin_emit(current_emit.size());
  vector<int> end_emit(current_emit_last.size());

  set_difference(current_emit.begin(), current_emit.end(),
                 current_emit_last.begin(), current_emit_last.end(),
                 begin_emit.begin());
  set_difference(current_emit_last.begin(), current_emit_last.end(),
                 current_emit.begin(), current_emit.end(),
                 end_emit.begin());

  begin_emit.erase(remove(begin_emit.begin(), begin_emit.end(), 0), begin_emit.end());
  end_emit.erase(remove(end_emit.begin(), end_emit.end(), 0), end_emit.end());

  if (begin_emit.size()) logQ("BEGIN EMIT:", begin_emit);
  if (end_emit.size())   logQ("END EMIT:", end_emit);

  vector<int> output;
  output.reserve(emitter_idx.size());
  set_difference(emitter_idx.begin(), emitter_idx.end(),
                 end_emit.begin(), end_emit.end(),
                 output.begin());
  swap(output, emitter_idx);

  emitter_idx.reserve(emitter_idx.size()+begin_emit.size());
  for (const auto b_idx : begin_emit) {
    emitter_idx.push_back(b_idx);
  }

  // ensure future process() calls do not need to sort emitter_idx twice
  std::sort(emitter_idx.begin(), emitter_idx.end());
}
