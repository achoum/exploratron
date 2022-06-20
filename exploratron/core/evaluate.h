#ifndef EXPLORATRON_CORE_EVALUATE_H_
#define EXPLORATRON_CORE_EVALUATE_H_

#include <vector>

#include "exploratron/core/abstract_arena.h"
#include "exploratron/core/abstract_controller.h"

namespace exploratron {

struct EvaluateOptions {
  int num_repetitions = 1;
  int step_sleep_ms = 0;
  bool display = true;
  bool pause = false;
};

Scores Evaluate(
    const AbstractArenaBuilder *arena_builder,
    const std::vector<const AbstractControllerBuilder *> &controller_builders,
    const EvaluateOptions &options = {});

void DisplayScores(const Scores &scores);

} // namespace exploratron

#endif