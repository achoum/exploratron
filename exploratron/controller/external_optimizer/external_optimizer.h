#ifndef CONTROLLER_EXTERNAL_OPTIMIZER_H_
#define CONTROLLER_EXTERNAL_OPTIMIZER_H_

#include "absl/types/span.h"
#include <memory>

#include "exploratron/core/abstract_controller.h"
#include "exploratron/core/utils/maths.h"
#include "exploratron/core/utils/neural_net.h"

namespace exploratron {
namespace external_optimizer {

struct Options {
  int population_size = 50; // 50 100
  size_t num_generations = 10000;
  int num_evaluation_repetitions = 20;

  std::vector<int> hidden_layers = {}; // 20

  std::string to_string() const {
    std::stringstream ss;
    ss << "pop-" << population_size;
    // ss << "_gen-" << num_generations;
    ss << "_evalr" << num_evaluation_repetitions;
    ss << "_h";
    for (const auto h : hidden_layers) {
      ss << "-" << h;
    }
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
  Genome Random();
  Genome Empty();

  MapDef map_definition_;
  std::mt19937 rnd_;
  neural_net::WeightsAllocator weigh_allocator;
  neural_net::WeightsAddress weight_address_1;
  neural_net::WeightsAddress weight_address_2;
  std::vector<neural_net::WeightsAddress> hidden_layers;
  const Options options_;
};

class ExternalOptimizerController : public AbstractController {
public:
  virtual ~ExternalOptimizerController() = default;
  ExternalOptimizerController(const Genome &genome,
                              const GenomeManager *genome_manager);
  Output Step(const Input &input) override;

  static constexpr int HISTORY_LENGTH = 8;

private:
  Genome genome_;
  std::vector<neural_net::VectorF> cache_hidden_;
  std::mt19937 rnd_;
  std::vector<int> last_dirs_;
  const GenomeManager *genome_manager_;
};

class ExternalOptimizerControllerBuilder : public AbstractControllerBuilder {
public:
  ExternalOptimizerControllerBuilder(const std::string_view path) {}
  ExternalOptimizerControllerBuilder(const Genome &genome,
                                     const GenomeManager *genome_manager)
      : genome_(genome), genome_manager_(genome_manager) {}
  virtual ~ExternalOptimizerControllerBuilder() = default;

  std::unique_ptr<AbstractController>
  Create(const MapDef &map_definition) const override {
    return std::make_unique<ExternalOptimizerController>(genome_,
                                                         genome_manager_);
  }

  std::string name() const override {
    return "ExternalOptimizerControllerBuilder";
  }

private:
  Genome genome_;
  const GenomeManager *genome_manager_;
};

} // namespace external_optimizer

REGISTER_AbstractControllerBuilder(
    external_optimizer::ExternalOptimizerControllerBuilder,
    ExternalOptimizerControllerBuilder, "ExternalOptimizer");
} // namespace exploratron

#endif