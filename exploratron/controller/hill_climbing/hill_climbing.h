#ifndef CONTROLLER_HILL_CLIMBING_H_
#define CONTROLLER_HILL_CLIMBING_H_

#include "absl/types/span.h"
#include <memory>

#include "exploratron/core/abstract_controller.h"
#include "exploratron/core/utils/maths.h"
#include "exploratron/core/utils/neural_net.h"

namespace exploratron {
namespace hill_climbing {

struct Options {
  float mutate_ratio = 0.1f;
  float init_weights = 0.1f;
  float mutate_scale = 0.05f;

  size_t num_iterations = 1000000;

  bool re_evaluate_best = true;

  std::string to_string() const {
    std::stringstream ss;
    ss << "sm-" << mutate_ratio << "x" << mutate_scale;
    return ss.str();
  }
};

struct Genome {
  bool operator<(const Genome &a) const { return fitness < a.fitness; }
  bool operator>(const Genome &a) const { return fitness > a.fitness; }

  float fitness = std::numeric_limits<float>::quiet_NaN();
  neural_net::VectorF weight_bank;
  MapDef map_definition;

  friend std::ostream &operator<<(std::ostream &stream, const Genome &g) {
    stream << print<float>(g.weight_bank);
    return stream;
  }
};

struct GenomeManager {
  GenomeManager(const MapDef &map_definition, const Options &options);

  Genome Mutate(const Genome &genome);
  Genome Random();

  MapDef map_definition_;
  std::mt19937 rnd_;
  neural_net::WeightsAllocator weigh_allocator;
  neural_net::WeightsAddress weight_address_1;
  neural_net::WeightsAddress weight_address_2;
  const Options options_;
};

class HillClimbingController : public AbstractController {
public:
  virtual ~HillClimbingController() = default;
  HillClimbingController(const Genome &genome,
                         const GenomeManager *genome_manager);
  Output Step(const Input &input) override;

  static constexpr int HISTORY_LENGTH = 3;

private:
  Genome genome_;
  neural_net::VectorF cache_hidden_1_;
  neural_net::VectorF cache_hidden_2_;
  std::mt19937 rnd_;
  std::vector<int> last_dirs_;
  const GenomeManager *genome_manager_;
};

class HillClimbingControllerBuilder : public AbstractControllerBuilder {
public:
  HillClimbingControllerBuilder(const std::string_view path) {}
  HillClimbingControllerBuilder(const Genome &genome,
                                const GenomeManager *genome_manager)
      : genome_(genome), genome_manager_(genome_manager) {}
  virtual ~HillClimbingControllerBuilder() = default;

  std::unique_ptr<AbstractController>
  Create(const MapDef &map_definition) const override {
    return std::make_unique<HillClimbingController>(genome_, genome_manager_);
  }

  std::string name() const override { return "HillClimbingControllerBuilder"; }

private:
  Genome genome_;
  const GenomeManager *genome_manager_;
};

} // namespace hill_climbing

REGISTER_AbstractControllerBuilder(hill_climbing::HillClimbingControllerBuilder,
                                   HillClimbingControllerBuilder,
                                   "HillClimbing");
} // namespace exploratron

#endif