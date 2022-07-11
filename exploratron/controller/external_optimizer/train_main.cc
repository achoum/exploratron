#include <iostream>

#include <execution>
#include <fstream>
#include <iostream>
#include <memory>
#include <signal.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"
#include "exploratron/controller/external_optimizer/external_optimizer.h"
#include "exploratron/core/abstract_arena.h"
#include "exploratron/core/abstract_controller.h"
#include "exploratron/core/evaluate.h"

ABSL_FLAG(std::string, arena, "Gather", "");
ABSL_FLAG(std::string, output_path, "", "");
ABSL_FLAG(std::string, training_log_base, "", "");

ABSL_FLAG(int, step_sleep_ms, 10, "");
ABSL_FLAG(bool, display, true, "");
ABSL_FLAG(int, num_threads, 8, "");

namespace exploratron {
namespace external_optimizer {

float EvaluateGenome(const AbstractArenaBuilder *arena_builder,
                     const Genome &genome, const EvaluateOptions &options,
                     const GenomeManager &genome_manager) {
  const auto controller_builder =
      ExternalOptimizerControllerBuilder(genome, &genome_manager);
  return Evaluate(arena_builder,
                  std::vector<const AbstractControllerBuilder *>{
                      &controller_builder},
                  options)
      .front();
}

struct Connection {

  std::string RawReadData() {
    char local_buffer[1024];
    while (true) {
      int i = buffer.find_first_of('\n');
      if (i != -1) {
        const auto ret = buffer.substr(0, i);
        buffer = buffer.substr(i + 1);
        return ret;
      }
      int num_read = read(inpipefd[0], local_buffer, sizeof(local_buffer));
      buffer += std::string_view(local_buffer, num_read);
    }
  }

  std::string ReadData() {
    while (true) {
      auto r = RawReadData();
      if (!r.empty() && r[0] == '#') {
        continue;
      }
      return r;
    }
  }

  void WriteData(std::string value) {
    value += "\n";
    write(outpipefd[1], value.c_str(), value.size());
  }

  void WriteData(int value) { WriteData(absl::StrCat(value)); }

  void Initialize() {
    pipe(inpipefd);
    pipe(outpipefd);
    pid = fork();
    if (pid == 0) {
      dup2(outpipefd[0], STDIN_FILENO);
      dup2(inpipefd[1], STDOUT_FILENO);
      // dup2(inpipefd[1], STDERR_FILENO);

      prctl(PR_SET_PDEATHSIG, SIGTERM);
      execl("/usr/bin/python3", "python3",
            "exploratron/controller/external_optimizer/optimizer_pgpelib.py",
            (char *)NULL);
      LOG(INFO) << "SHOULD NOT HAPPEN";
      exit(1);
    }
    close(outpipefd[0]);
    close(inpipefd[1]);
  }

  void Stop() {
    LOG(INFO) << "Kill connection";
    kill(pid, SIGKILL); // send SIGKILL signal to the child process
    waitpid(pid, &status, 0);
  }

  int inpipefd[2];
  int outpipefd[2];
  pid_t pid;
  int status;
  std::string buffer;
};

void ParseVector(const std::string &in, std::vector<float> *output) {
  std::vector<std::string> raw_ins = absl::StrSplit(in, " ");
  output->clear();
  output->reserve(raw_ins.size());
  for (const auto &raw_in : raw_ins) {
    output->push_back(std::stof(raw_in));
  }
}

void TrainExternalOptimizer() {
  // Select the working arena.
  const auto arena_builder =
      AbstractArenaBuilderRegisterer::Create(absl::GetFlag(FLAGS_arena));

  Options options;

  // Display during the learning.
  // Disable display and sleeping for maximum speed.

  EvaluateOptions training_options;
  training_options.num_repetitions = options.num_evaluation_repetitions;
  training_options.step_sleep_ms = 0;
  training_options.display = false;

  EvaluateOptions evaluation_options;
  evaluation_options.num_repetitions = 100;
  evaluation_options.step_sleep_ms = 100;
  evaluation_options.display = true;
  evaluation_options.pause = false;

  const auto map_definition = arena_builder->MapDefinition();

  GenomeManager genome_manager(map_definition, options);

  Connection connection;
  connection.Initialize();

  connection.WriteData(genome_manager.weigh_allocator.size());
  connection.WriteData(options.population_size);

  size_t log_every = 10;

  std::unique_ptr<std::ofstream> output_file;
  if (!absl::GetFlag(FLAGS_training_log_base).empty()) {
    std::string log_path =
        absl::GetFlag(FLAGS_training_log_base) + options.to_string() + ".csv";
    output_file = std::make_unique<std::ofstream>();
    output_file->open(log_path);
    (*output_file) << "generation,max_fitness\n";
  }

  std::vector<float> weights;
  std::vector<double> evaluations;

  std::vector<int> sequence(options.population_size); // TODO: Improve.
  std::iota(sequence.begin(), sequence.end(), 0);

  Genome best_genome;

  double sum_last_log = 0;
  int count_last_log = 0;

  for (int generation_idx = 0; generation_idx < options.num_generations;
       generation_idx++) {

    const auto raw_candidates = connection.ReadData();
    // LOG(INFO) << "raw_candidates:" << raw_candidates;
    std::vector<std::string> rows = absl::StrSplit(raw_candidates, ",");
    evaluations.resize(rows.size());
    std::for_each_n(
        std::execution::par, sequence.begin(), sequence.size(), [&](int index) {
          Genome genome = genome_manager.Empty();
          ParseVector(rows[index], &genome.weight_bank);
          evaluations[index] = EvaluateGenome(arena_builder.get(), genome,
                                              training_options, genome_manager);
        });

    const auto raw_evaluations = absl::StrJoin(evaluations, " ");
    // LOG(INFO) << "raw_evaluations:" << raw_evaluations;
    connection.WriteData(raw_evaluations);

    const auto raw_center = connection.ReadData();
    // LOG(INFO) << "raw_center:" << raw_center;
    Genome center_genome = genome_manager.Empty();
    ParseVector(raw_center, &center_genome.weight_bank);
    center_genome.fitness = EvaluateGenome(arena_builder.get(), center_genome,
                                           training_options, genome_manager);

    // if (std::isnan(best_genome.fitness) ||
    //     center_genome.fitness > best_genome.fitness) {
    best_genome = center_genome;
    //}

    sum_last_log += center_genome.fitness;
    count_last_log++;

    if (output_file) {
      (*output_file) << generation_idx << "," << center_genome.fitness << "\n";
    }
    if ((generation_idx % log_every) == 0) {
      if (output_file) {
        output_file->flush();
      }
      double mean_fitness = sum_last_log / count_last_log;
      sum_last_log = 0;
      count_last_log = 0;
      LOG(INFO) << "Generation #" << generation_idx
                << " last_fitness: " << center_genome.fitness
                << " mean_fitness: " << mean_fitness;
    }
  }

  connection.Stop();

  if (output_file) {
    output_file->close();
  }

  LOG(INFO) << "Run with the best genome: " << best_genome.fitness << " : ";
  EvaluateGenome(arena_builder.get(), best_genome, evaluation_options,
                 genome_manager);
}

} // namespace external_optimizer
} // namespace exploratron

int main(int argc, char **argv) {
  absl::ParseCommandLine(argc, argv);
  exploratron::external_optimizer::TrainExternalOptimizer();
}