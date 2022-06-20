#include "exploratron/controller/random/random.h"
#include "exploratron/core/utils/logging.h"

namespace exploratron {
namespace random {
RandomController::RandomController() : rnd_(std::random_device{}()) {}

Output RandomController::Step(const Input& input) {
  Output output;

  std::uniform_int_distribution<int> dir_dist(0, 4);
  output.move = static_cast<eDirection>(dir_dist(rnd_));

  return output;
}

} // namespace random
} // namespace exploratron