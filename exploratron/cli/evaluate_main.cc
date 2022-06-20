#include <iostream>

#include <memory>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

#include "exploratron/core/abstract_arena.h"
#include "exploratron/core/abstract_controller.h"
#include "exploratron/core/evaluate.h"

ABSL_FLAG(std::string, arena, "Gather", "");
ABSL_FLAG(std::string, controller_key, "Random", "");
ABSL_FLAG(std::string, controller_path, "", "");
ABSL_FLAG(int, step_sleep_ms, 100, "");
ABSL_FLAG(int, num_repetitions, 1, "");
ABSL_FLAG(bool, display, true, "");

namespace exploratron {

void Evaluate() {

  const auto arena_builder =
      AbstractArenaBuilderRegisterer::Create(absl::GetFlag(FLAGS_arena));
  const auto controller_builder = AbstractControllerBuilderRegisterer::Create(
      absl::GetFlag(FLAGS_controller_key),
      absl::GetFlag(FLAGS_controller_path));

  EvaluateOptions options;
  options.step_sleep_ms = absl::GetFlag(FLAGS_step_sleep_ms);
  options.num_repetitions = absl::GetFlag(FLAGS_num_repetitions);
  options.display = absl::GetFlag(FLAGS_display);

  const auto scores = Evaluate(
      arena_builder.get(),
      std::vector<const AbstractControllerBuilder *>{controller_builder.get()},
      options);

  DisplayScores(scores);
}

} // namespace exploratron

int main(int argc, char **argv) {
  absl::ParseCommandLine(argc, argv);
  exploratron::Evaluate();
}