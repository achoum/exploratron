#ifndef CONTROLLER_GENETIC_H_
#define CONTROLLER_GENETIC_H_

#include "absl/types/span.h"
#include <memory>

#include "exploratron/core/abstract_controller.h"
#include "exploratron/core/utils/maths.h"
#include "exploratron/core/utils/neural_net.h"

namespace exploratron {
namespace genetic {

struct Options {

  enum class eCrossOver {
    kOnePoint,
    kTwoPoints,
    kNone,
    kRandom,
  };
  eCrossOver cross_over = eCrossOver::kOnePoint;
  float cross_over_rate = 0.9;
  float cross_over_random_balance = 0.5;

  enum class eMutate {
    kSome,
    kAll,
  };
  eMutate mutate = eMutate::kAll;
  float mutate_ratio = 0.10f;
  float mutate_scale = 0.03f;

  int population_size = 100;
  size_t num_generations = 5000;
  int num_elite = 5;
  int num_random = 10;

  float tournament_ratio = 0.1f;

  int num_repetitions = 5;

  std::vector<int> hidden_layers = {20};

  std::string to_string() const {
    std::stringstream ss;

    ss << "pop-" << population_size;
    ss << "_elit-" << num_elite;
    ss << "_to-ra-" << tournament_ratio;

    // Cross-over
    ss << "_cross";
    switch (cross_over) {
    case eCrossOver::kNone:
      ss << "no";
      break;
    case eCrossOver::kOnePoint:
      ss << "1p-r-" << cross_over_rate;
      break;
    case eCrossOver::kTwoPoints:
      ss << "2p-r-" << cross_over_rate;
      break;
    case eCrossOver::kRandom:
      ss << "rnd-r-" << cross_over_random_balance << "x" << cross_over_rate;
      break;
    }

    // Mutate
    ss << "_mut-";
    switch (mutate) {
    case eMutate::kAll:
      ss << "all-" << mutate_scale;
      break;
    case eMutate::kSome:
      ss << "sm-" << mutate_ratio << "x" << mutate_scale;
      break;
    }

    if (!hidden_layers.empty()) {
      ss << "_h";
      for (const auto n : hidden_layers) {
        ss << "-" << n;
      }
    }

    ss << "_rep-" << num_repetitions;
    return ss.str();
  }
};

struct Genome {
  bool operator<(const Genome &a) const { return fitness < a.fitness; }
  bool operator>(const Genome &a) const { return fitness > a.fitness; }
  friend std::ostream &operator<<(std::ostream &stream, const Genome &g) {
    stream << print<float>(g.weight_bank);
    return stream;
  }

  float fitness = std::numeric_limits<float>::quiet_NaN();
  neural_net::VectorF weight_bank;
  MapDef map_definition;
};

struct GenomeManager {
  GenomeManager(const MapDef &map_definition, const Options &options);
  void Mutate(Genome *genome);
  Genome Crossover(const Genome &a, const Genome &b);
  Genome Random();
  void PointMutate(float ratio, std::vector<float> *vs);
  const Genome &SelectIndividual(const std::vector<Genome> &population);

  MapDef map_definition_;
  std::mt19937 rnd_;
  neural_net::WeightsAllocator weigh_allocator;
  neural_net::WeightsAddress weight_address_1;
  neural_net::WeightsAddress weight_address_2;
  std::vector<neural_net::WeightsAddress> hidden_layers;
  const Options options_;
};

class GeneticController : public AbstractController {
public:
  virtual ~GeneticController() = default;
  GeneticController(const Genome &genome, const GenomeManager *genome_manager);
  Output Step(const Input &input) override;

  static constexpr int HISTORY_LENGTH = 8;

private:
  Genome genome_;
  std::vector<neural_net::VectorF> cache_hidden_;
  std::mt19937 rnd_;
  std::vector<int> last_dirs_;
  const GenomeManager *genome_manager_;
};

class GeneticControllerBuilder : public AbstractControllerBuilder {
public:
  GeneticControllerBuilder(const std::string_view path) {}
  GeneticControllerBuilder(const Genome &genome,
                           const GenomeManager *genome_manager)
      : genome_(genome), genome_manager_(genome_manager) {}
  virtual ~GeneticControllerBuilder() = default;

  std::unique_ptr<AbstractController>
  Create(const MapDef &map_definition) const override {
    return std::make_unique<GeneticController>(genome_, genome_manager_);
  }

  std::string name() const override { return "GeneticControllerBuilder"; }

private:
  Genome genome_;
  const GenomeManager *genome_manager_;
};

} // namespace genetic

REGISTER_AbstractControllerBuilder(genetic::GeneticControllerBuilder,
                                   GeneticControllerBuilder, "Genetic");
} // namespace exploratron

#endif