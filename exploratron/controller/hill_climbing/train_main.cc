#include <iostream>

#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

#include "exploratron/core/abstract_arena.h"
#include "exploratron/core/abstract_controller.h"
#include "exploratron/core/evaluate.h"
#include "exploratron/controller/hill_climbing/hill_climbing.h"

ABSL_FLAG(std::string, arena, "Gather", "");
ABSL_FLAG(std::string, output_path, "", "");
ABSL_FLAG(std::string, training_log_base, "", "");

namespace exploratron {
namespace hill_climbing {

float Evaluate(const AbstractArenaBuilder *arena_builder, const Genome &genome,
               const EvaluateOptions &options,
               const GenomeManager &genome_manager) {
  const auto controller_builder =
      HillClimbingControllerBuilder(genome, &genome_manager);
  return Evaluate(arena_builder,
                  std::vector<const AbstractControllerBuilder *>{
                      &controller_builder},
                  options)
      .front();
}

void Train() {
  // Select the working arena.
  const auto arena_builder =
      AbstractArenaBuilderRegisterer::Create(absl::GetFlag(FLAGS_arena));

  // Display during the learning.
  // Disable display and sleeping for maximum speed.
  EvaluateOptions training_options;
  training_options.num_repetitions = 4;
  training_options.step_sleep_ms = 0;
  training_options.display = false;

  EvaluateOptions evaluation_options;
  evaluation_options.num_repetitions = 100;
  evaluation_options.step_sleep_ms = 100;
  evaluation_options.display = true;
  evaluation_options.pause = false;

  // Initiate search
  Options options;
  const auto map_definition = arena_builder->MapDefinition();
  GenomeManager genome_manager(map_definition, options);
  Genome genome = genome_manager.Random();
  size_t log_every = 1000;

  // Loggin
  std::unique_ptr<std::ofstream> output_file;
  if (!absl::GetFlag(FLAGS_training_log_base).empty()) {
    std::string log_path =
        absl::GetFlag(FLAGS_training_log_base) + options.to_string() + ".csv";
    output_file = std::make_unique<std::ofstream>();
    output_file->open(log_path);
    (*output_file) << "iteration,fitness,best_fitness,num_better_candidates,"
                      "num_equal_candidates\n";
  }

  size_t num_better_candidates = 0;
  size_t num_equal_candidates = 0;
  size_t iteration_idx = 0;
  float best_fitness = -1;
  float fitness =
      Evaluate(arena_builder.get(), genome, training_options, genome_manager);
  LOG(INFO) << "Initial fitness: " << fitness;

  for (size_t iteration_idx = 0; iteration_idx < options.num_iterations;
       iteration_idx++) {

    // Re-evaluate because of stocastic evaluation
    if (options.re_evaluate_best) {
      fitness = Evaluate(arena_builder.get(), genome, training_options,
                         genome_manager);
    }

    // New candidate
    auto candidate = genome_manager.Mutate(genome);
    // auto candidate = genome_manager.Random();
    // LOG(INFO) << candidate;

    auto candidate_fitness = Evaluate(arena_builder.get(), candidate,
                                      training_options, genome_manager);

    if (candidate_fitness >= fitness) {
      if (candidate_fitness > fitness) {
        num_better_candidates++;
      } else {
        num_equal_candidates++;
      }
      fitness = candidate_fitness;
      genome = candidate;
    }

    if (fitness > best_fitness) {
      best_fitness = fitness;
    }

    // Logging
    if (output_file) {
      (*output_file) << iteration_idx << "," << fitness << "," << best_fitness
                     << "," << num_better_candidates << ","
                     << num_equal_candidates << "\n";
    }
    if ((iteration_idx % log_every) == 0) {
      if (output_file) {
        output_file->flush();
      }
      LOG(INFO) << "Iteration #" << iteration_idx << " fitness: " << fitness
                << " better:" << num_better_candidates
                << " equal:" << num_equal_candidates
                << " best_fitness:" << best_fitness
                << " candidate_fitness:" << candidate_fitness;
    }
  }

  if (output_file) {
    output_file->close();
  }
  Evaluate(arena_builder.get(), genome, evaluation_options, genome_manager);
}

} // namespace hill_climbing
} // namespace exploratron

int main(int argc, char **argv) {
  absl::ParseCommandLine(argc, argv);
  exploratron::hill_climbing::Train();
}