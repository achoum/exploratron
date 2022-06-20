#ifndef EXPLORATRON_CONTROLLER_GENETIC_H_
#define EXPLORATRON_CONTROLLER_GENETIC_H_

#include <memory>
#include <random>

#include "exploratron/core/abstract_controller.h"

namespace exploratron {
namespace random {
class RandomController : public AbstractController {
public:
  virtual ~RandomController() = default;
  RandomController();
  Output Step(const Input& input) override;

private:
  std::mt19937_64 rnd_;
};

class RandomControllerBuilder : public AbstractControllerBuilder {
public:
  RandomControllerBuilder(const std::string_view path) {}
  virtual ~RandomControllerBuilder() = default;

  std::unique_ptr<AbstractController>
  Create(const MapDef &map_definition) const override {
    return std::make_unique<RandomController>();
  }

  std::string name() const override { return "RandomControllerBuilder"; }
};
} // namespace random

REGISTER_AbstractControllerBuilder(random::RandomControllerBuilder,
                                   RandomControllerBuilder, "Random");

} // namespace exploratron

#endif