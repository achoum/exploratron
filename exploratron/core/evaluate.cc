#include "exploratron/core/evaluate.h"

#include "exploratron/core/utils/logging.h"
#include "exploratron/core/utils/terminal.h"
#include <algorithm>
#include <assert.h>
#include <chrono>
#include <iostream>
#include <thread>

namespace exploratron {

Scores Evaluate(
    const AbstractArenaBuilder *arena_builder,
    const std::vector<const AbstractControllerBuilder *> &controller_builders,
    const EvaluateOptions &options) {

  if (options.display) {
    terminal::Initialize();
  }

  Scores scores = {};

  for (int repetition_idx = 0; repetition_idx < options.num_repetitions;
       repetition_idx++) {
    // Create area.
    auto area = arena_builder->Create(controller_builders);

    // Run area.
    while (true) {
      if (options.display) {
        terminal::ClearScreen();
        area->Draw();
        terminal::RefreshScreen();
      }
      if (!area->Step()) {
        break;
      }
      if (options.step_sleep_ms > 0) {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(options.step_sleep_ms));
      }
      if (options.pause) {
        LOG(INFO) << "---Press enter to continue--";
        std::getchar();
      }
    }

    // Merge scores.
    const auto sub_scores = area->FinalScore();
    if (repetition_idx == 0) {
      scores = sub_scores;
    } else {
      DCHECK_EQ(scores.size(), sub_scores.size());
      std::transform(sub_scores.begin(), sub_scores.end(), scores.begin(),
                     scores.begin(), std::plus<float>());
    }
  }

  // Normalize scores.
  for (auto &score : scores) {
    score /= options.num_repetitions;
  }

  if (options.display) {
    terminal::Uninitialize();
  }

  return scores;
}

void DisplayScores(const Scores &scores) {
  LOG(INFO) << "Score:";
  for (const auto score : scores) {
    LOG(INFO) << " " << score;
  }
}

} // namespace exploratron