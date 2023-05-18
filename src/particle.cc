#include "particle.h"
#include "log.h"

using std::remove_if;
using std::make_pair;
using std::set_difference;

void particleController::update() {
  current_emit_last = current_emit;
  current_emit.clear();

  for (auto& em : emitter_map) {
    em.second.update_part(GetFrameTime());
  }
}
    
void particleController::add_emitter(int index, const particleInfo& p_info) {
  current_emit.push_back(make_pair(index, p_info));
}
    
void particleController::end_emission() {
  current_emit.clear();
  process();
}
    
void particleController::process() {
  vector<pair<int, particleInfo>> begin_emit(current_emit.size());
  vector<pair<int, particleInfo>> end_emit(current_emit_last.size());

  set_difference(current_emit.begin(), current_emit.end(),
                 current_emit_last.begin(), current_emit_last.end(),
                 begin_emit.begin(), set_comp());
  set_difference(current_emit_last.begin(), current_emit_last.end(),
                 current_emit.begin(), current_emit.end(),
                 end_emit.begin(), set_comp());

  auto zero_comp = [&](const auto& a) {
    return a.first == 0;
  };

  begin_emit.erase(remove_if(begin_emit.begin(), begin_emit.end(), zero_comp), begin_emit.end());
  end_emit.erase(remove_if(end_emit.begin(), end_emit.end(), zero_comp), end_emit.end());

  if (begin_emit.size()) logQ("BEGIN EMIT:", begin_emit);
  if (end_emit.size())   logQ("END EMIT:", end_emit);


  for (const auto& e_idx : end_emit) {
    auto e_it = emitter_map.find(e_idx);
    if (e_it != emitter_map.end()) {
      emitter_map.erase(e_it);
    }
  }

  // update emitter metadata for still-existing emitters
  for (const auto& c_idx : current_emit) {
    auto em_it = emitter_map.find(c_idx);
    if (em_it != emitter_map.end()) {
      em_it->second.update_data(c_idx.second);
    }
  }

  for (const auto& b_idx : begin_emit) {
    auto b_it = emitter_map.find(b_idx);
    if (b_it == emitter_map.end()) {
      emitter e = emitter();
      e.init(b_idx);
      emitter_map.insert(make_pair(b_idx, std::move(e)));
    }
  }
}
    
void particleController::render() {
  for (auto& em : emitter_map) {
    em.second.render();
  }
}
