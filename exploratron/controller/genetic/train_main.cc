#include <iostream>

#include <execution>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

#include "exploratron/core/abstract_arena.h"
#include "exploratron/core/abstract_controller.h"
#include "exploratron/core/evaluate.h"
#include "exploratron/controller/genetic/genetic.h"

ABSL_FLAG(std::string, arena, "Gather", "");
ABSL_FLAG(std::string, output_path, "", "");
ABSL_FLAG(std::string, training_log_base, "", "");

ABSL_FLAG(int, step_sleep_ms, 10, "");
ABSL_FLAG(int, num_repetitions, 100, "");
ABSL_FLAG(bool, display, true, "");
ABSL_FLAG(int, num_threads, 8, "");

namespace exploratron {
namespace genetic {

float EvaluateGenome(const AbstractArenaBuilder *arena_builder,
                     const Genome &genome, const EvaluateOptions &options,
                     const GenomeManager &genome_manager) {
  const auto controller_builder =
      GeneticControllerBuilder(genome, &genome_manager);
  return Evaluate(arena_builder,
                  std::vector<const AbstractControllerBuilder *>{
                      &controller_builder},
                  options)
      .front();
}

void TrainGenetic() {
  // Select the working arena.
  const auto arena_builder =
      AbstractArenaBuilderRegisterer::Create(absl::GetFlag(FLAGS_arena));

  Options options;

  // Display during the learning.
  // Disable display and sleeping for maximum speed.

  EvaluateOptions training_options;
  training_options.num_repetitions = options.num_repetitions;
  training_options.step_sleep_ms = 0;
  training_options.display = false;

  EvaluateOptions evaluation_options;
  evaluation_options.num_repetitions = 100;
  evaluation_options.step_sleep_ms = 100;
  evaluation_options.display = true;
  evaluation_options.pause = false;

  const auto map_definition = arena_builder->MapDefinition();

  GenomeManager genome_manager(map_definition, options);

  const int num_mutate_and_reproduce =
      options.population_size - options.num_elite - options.num_random;

  LOG(INFO) << "Initialize population";
  std::vector<Genome> population;
  std::vector<Genome> next_population;
  population.reserve(options.population_size);
  next_population.reserve(options.population_size);

  for (int i = 0; i < options.population_size; i++) {
    population.push_back(genome_manager.Random());
  }

  size_t log_every = 100;

  std::unique_ptr<std::ofstream> output_file;
  if (!absl::GetFlag(FLAGS_training_log_base).empty()) {
    std::string log_path =
        absl::GetFlag(FLAGS_training_log_base) + options.to_string() + ".csv";
    output_file = std::make_unique<std::ofstream>();
    output_file->open(log_path);
    (*output_file) << "generation,max_fitness,median_fitness,min_fitness\n";
  }

  size_t generation_idx = 0;
  while (options.num_generations != 0) {

    /*
    for (auto &genome : population) {
      genome.fitness = EvaluateGenome(arena_builder.get(), genome,
                                      training_options, genome_manager);
    }*/
    std::for_each(std::execution::par, population.begin(), population.end(),
                  [&](auto &genome) {
                    genome.fitness =
                        EvaluateGenome(arena_builder.get(), genome,
                                       training_options, genome_manager);
                  });

    std::sort(population.begin(), population.end(), std::greater<Genome>());

    auto max_fitness = population.front().fitness;
    auto median_fitness = population[population.size() / 2].fitness;
    auto min_fitness = population.back().fitness;

    if (output_file) {
      (*output_file) << generation_idx << "," << max_fitness << ","
                     << median_fitness << "," << min_fitness << "\n";
    }
    if ((generation_idx % log_every) == 0) {
      if (output_file) {
        output_file->flush();
      }

      LOG(INFO) << "Generation #" << generation_idx
                << " max_fitness: " << max_fitness
                << " med_fitness:" << median_fitness
                << " min_fitness:" << min_fitness
                << " population:" << population.size();
    }

    if (generation_idx >= options.num_generations - 1) {
      break;
    }

    next_population.clear();
    // Elite
    for (int i = 0; i < options.num_elite; i++) {
      next_population.push_back(population[i]);
    }

    // Normal
    for (int i = 0; i < num_mutate_and_reproduce; i++) {
      const auto &a = genome_manager.SelectIndividual(population);
      const auto &b = genome_manager.SelectIndividual(population);

      auto c = genome_manager.Crossover(a, b);
      genome_manager.Mutate(&c);
      next_population.push_back(c);
    }

    // Random.
    for (int i = 0; i < options.num_random; i++) {
      next_population.push_back(genome_manager.Random());
    }

    population = std::move(next_population);
    generation_idx++;
  }

  if (output_file) {
    output_file->close();
  }

  LOG(INFO) << "Best genome: " << population.front().fitness << " : "
            << population.front();

  EvaluateGenome(arena_builder.get(), population.front(), evaluation_options,
                 genome_manager);
}

} // namespace genetic
} // namespace exploratron

int main(int argc, char **argv) {
  absl::ParseCommandLine(argc, argv);
  exploratron::genetic::TrainGenetic();
}