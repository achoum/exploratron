#include "exploratron/controller/hill_climbing/hill_climbing.h"
#include "exploratron/core/utils/logging.h"
#include "exploratron/core/utils/macros.h"

#include <assert.h>
#include <random>
#include <stdint.h>

namespace exploratron {
namespace hill_climbing {

constexpr int Numdir() { return static_cast<int>(eDirection::_NUM_DIRECTIONS); }

GenomeManager::GenomeManager(const MapDef &map_definition,
                             const Options &options)
    : map_definition_(map_definition), options_(options) {

  weight_address_1 = weigh_allocator.CreateAddress(
      map_definition_.shape.Size() * map_definition_.num_values, Numdir());

  weight_address_2 = weigh_allocator.CreateAddress(
      Numdir() * HillClimbingController::HISTORY_LENGTH, Numdir());

  LOG(INFO) << "Terrain weights: " << weight_address_1.size;
  LOG(INFO) << "Past weights: " << weight_address_2.size;
  LOG(INFO) << "Total number of weights: " << weigh_allocator.next_begin;

  InitRandom(&rnd_);
}

Genome GenomeManager::Mutate(const Genome &genome) {
  Genome candidate = genome;
  auto &data = candidate.weight_bank;

  auto noise_distribution = std::uniform_real_distribution<float>(
      -options_.mutate_scale, options_.mutate_scale);
  FOR_I(data.size()) { data[i] += noise_distribution(rnd_); }

  /*
  const int k =
      std::binomial_distribution<int>(data.size(), options_.mutate_ratio)(rnd_);
  auto dist_a = std::uniform_real_distribution<float>(-options_.mutate_scale,
                                                      options_.mutate_scale);
  FOR_I(k) {
    const int j = RND_UNIF_INT(data.size(), rnd_);
    const float v = dist_a(rnd_);
    data[j] += v;
  }
  */
  /*
    const int k = data.size() * options_.mutate_ratio;
    assert(k > 0);
    auto dist_a = std::uniform_real_distribution<float>(-options_.mutate_scale,
                                                        options_.mutate_scale);
    FOR_I(k) {
      const int j = RND_UNIF_INT(data.size(), rnd_);
      data[j] += dist_a(rnd_);
    }*/

  /*
    auto dist_a = std::uniform_real_distribution<float>(-options_.mutate_scale,
                                                        options_.mutate_scale);
    FOR_I(1) {
      const int j = RND_UNIF_INT(data.size(), rnd_);
      data[j] += dist_a(rnd_);
    }
  */

  // neural_net::BoundValues(5, &data);

  return candidate;
}

Genome GenomeManager::Random() {
  Genome child;
  child.map_definition = map_definition_;
  weigh_allocator.AllocateWeightBank(&child.weight_bank);
  neural_net::InitWeights(&child.weight_bank, &rnd_, options_.init_weights,
                          neural_net::eEnumWeights::SYMETRICAL_UNIFORM);
  return child;
}

HillClimbingController::HillClimbingController(
    const Genome &genome, const GenomeManager *genome_manager)
    : genome_(genome), genome_manager_(genome_manager) {
  cache_hidden_1_.resize(Numdir());
  cache_hidden_2_.resize(Numdir());
  last_dirs_.assign(HISTORY_LENGTH, 0);

  InitRandom(&rnd_);
}

Output HillClimbingController::Step(const Input &input) {
  neural_net::BoundValues(2, &genome_.weight_bank);

  // Perfect automaton.

  // LOG(INFO) << print<float>(input.surouding.values);
  /*std::fill(genome_.weight_bank.begin(), genome_.weight_bank.end(), 0);
  {
    auto tmp = genome_manager_->weight_address_1(genome_.weight_bank);
    neural_net::SetWeightOneHotConstant(
        genome_manager_->map_definition_.shape.Size(),
        genome_manager_->map_definition_.num_values, Numdir(),
        static_cast<int>(eDirection::RIGHT), 0.5, &tmp);

    neural_net::SetWeightOneHot(genome_manager_->map_definition_.shape.Size(),
                                genome_manager_->map_definition_.num_values,
                                Numdir(), 5, 1,
                                static_cast<int>(eDirection::UP), 1, &tmp);
  }*/

  neural_net::ForwardOneHot<neural_net::Identity>(
      input.surouding.values, genome_.map_definition.num_values,
      genome_manager_->weight_address_1(genome_.weight_bank), &cache_hidden_1_);

  neural_net::ForwardOneHot<neural_net::Identity>(
      last_dirs_, Numdir(),
      genome_manager_->weight_address_2(genome_.weight_bank), &cache_hidden_2_);

  neural_net::AddTo<neural_net::Relu>(cache_hidden_1_, &cache_hidden_2_);

  int dir = neural_net::SoftMaxSampling(cache_hidden_2_, &rnd_);
  // int dir = neural_net::ArgMax(cache_hidden_2_);
  //  LOG(INFO) << print<float>(cache_hidden_2_) << " -> " << dir;

  Output output;
  output.move = static_cast<eDirection>(dir);

  // Rememeber last directions.
  FOR_I(last_dirs_.size() - 1) { last_dirs_[i] = last_dirs_[i + 1]; }
  last_dirs_.back() = dir;
  return output;
}

} // namespace hill_climbing
} // namespace exploratron