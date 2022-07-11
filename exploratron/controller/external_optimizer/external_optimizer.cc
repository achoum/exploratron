#include "exploratron/controller/external_optimizer/external_optimizer.h"
#include "exploratron/core/utils/logging.h"
#include "exploratron/core/utils/macros.h"

#include <assert.h>
#include <random>
#include <stdint.h>

namespace exploratron {
namespace external_optimizer {

constexpr int Numdir() { return static_cast<int>(eDirection::_NUM_DIRECTIONS); }

GenomeManager::GenomeManager(const MapDef &map_definition,
                             const Options &options)
    : map_definition_(map_definition), options_(options) {

  LOG(INFO) << "Create genome manager on:";
  LOG(INFO) << "\tmap size:" << map_definition_.shape;
  LOG(INFO) << "\tnum possible values:" << (int)map_definition_.num_values;
  LOG(INFO) << "\tnum output dirs:" << Numdir();
  LOG(INFO) << "\thistory:" << ExternalOptimizerController::HISTORY_LENGTH;

  if (options.hidden_layers.empty()) {
    weight_address_1 = weigh_allocator.CreateAddress(
        map_definition_.shape.Size() * map_definition_.num_values, Numdir());
    weight_address_2 = weigh_allocator.CreateAddress(
        Numdir() * ExternalOptimizerController::HISTORY_LENGTH, Numdir());
  } else {
    int n = options.hidden_layers.front();
    weight_address_1 = weigh_allocator.CreateAddress(
        map_definition_.shape.Size() * map_definition_.num_values, n);
    weight_address_2 = weigh_allocator.CreateAddress(
        Numdir() * ExternalOptimizerController::HISTORY_LENGTH, n);

    for (int i = 1; i < options.hidden_layers.size(); i++) {
      const auto next_n = options.hidden_layers[i];
      hidden_layers.push_back(weigh_allocator.CreateAddress(n, next_n));
      n = next_n;
    }
    hidden_layers.push_back(weigh_allocator.CreateAddress(n, Numdir()));
  }

  LOG(INFO) << "Terrain weights: " << weight_address_1.size;
  LOG(INFO) << "Past weights: " << weight_address_2.size;
  LOG(INFO) << "Total number of weights: " << weigh_allocator.next_begin;
}

Genome GenomeManager::Random() {
  Genome child;
  child.map_definition = map_definition_;

  weigh_allocator.AllocateWeightBank(&child.weight_bank);
  neural_net::InitWeights(&child.weight_bank, &rnd_);
  return child;
}

Genome GenomeManager::Empty() {
  Genome child;
  child.map_definition = map_definition_;

  weigh_allocator.AllocateWeightBank(&child.weight_bank);
  return child;
}

ExternalOptimizerController::ExternalOptimizerController(
    const Genome &genome, const GenomeManager *genome_manager)
    : genome_(genome), genome_manager_(genome_manager) {

  if (genome_manager->options_.hidden_layers.empty()) {
    cache_hidden_.push_back(neural_net::VectorF(Numdir()));
    cache_hidden_.push_back(neural_net::VectorF(Numdir()));
  } else {
    cache_hidden_.push_back(
        neural_net::VectorF(genome_manager->options_.hidden_layers.front()));
    for (const auto n : genome_manager->options_.hidden_layers) {
      cache_hidden_.push_back(neural_net::VectorF(n));
    }
    cache_hidden_.push_back(neural_net::VectorF(Numdir()));
  }

  last_dirs_.assign(HISTORY_LENGTH, 0);
}

Output ExternalOptimizerController::Step(const Input &input) {
  neural_net::ForwardOneHot<neural_net::Identity>(
      input.surouding.values, genome_.map_definition.num_values,
      genome_manager_->weight_address_1(genome_.weight_bank),
      &cache_hidden_[0]);

  neural_net::ForwardOneHot<neural_net::Identity>(
      last_dirs_, Numdir(),
      genome_manager_->weight_address_2(genome_.weight_bank),
      &cache_hidden_[1]);

  if (!genome_manager_->options_.hidden_layers.empty()) {
    neural_net::AddTo<neural_net::Relu>(cache_hidden_[0],
                                            &cache_hidden_[1]);

    for (int i = 0; i < genome_manager_->options_.hidden_layers.size() - 1;
         i++) {
      neural_net::Forward<neural_net::Relu>(
          cache_hidden_[i + 1],
          genome_manager_->hidden_layers[i](genome_.weight_bank),
          &cache_hidden_[i + 2]);
    }

    const int i = genome_manager_->options_.hidden_layers.size() - 1;
    neural_net::Forward<neural_net::Identity>(
        cache_hidden_[i + 1],
        genome_manager_->hidden_layers[i](genome_.weight_bank),
        &cache_hidden_[i + 2]);
  } else {
    neural_net::AddTo<neural_net::Identity>(cache_hidden_[0],
                                            &cache_hidden_[1]);
  }

  int dir = neural_net::SoftMaxSampling(cache_hidden_.back(), &rnd_);
  // int dir = neural_net::ArgMax(cache_hidden_.back());

  /*
    LOG(INFO) << "Inputs:\n" << input.surouding;

    LOG(INFO) << "weights 1:"
              << print<float>(
                     genome_manager_->weight_address_1(genome_.weight_bank));

    LOG(INFO) << "weights 2:"
              << print<float>(
                     genome_manager_->weight_address_2(genome_.weight_bank));

    LOG(INFO) << "hidden 1:" << print<float>(cache_hidden_1_);
    LOG(INFO) << "hidden 2:" << print<float>(cache_hidden_2_);
    LOG(INFO) << "dir:" << dir;
  */

  Output output;
  output.move = static_cast<eDirection>(dir);
  FOR_I(last_dirs_.size() - 1) { last_dirs_[i] = last_dirs_[i + 1]; }
  last_dirs_.back() = dir;
  return output;
}

} // namespace external_optimizer
} // namespace exploratron