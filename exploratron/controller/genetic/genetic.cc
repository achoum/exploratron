#include "exploratron/controller/genetic/genetic.h"
#include "exploratron/core/utils/logging.h"
#include "exploratron/core/utils/macros.h"

#include <assert.h>
#include <random>
#include <stdint.h>

namespace exploratron {
namespace genetic {

constexpr int Numdir() { return static_cast<int>(eDirection::_NUM_DIRECTIONS); }

void MutationRandomExp(const float rate, const float scale,
                       std::vector<float> *data, std::mt19937 *rnd) {
  const int k = std::binomial_distribution<int>(data->size(), rate)(*rnd);
  auto dist_a = std::uniform_real_distribution<float>(0, 10);
  auto dist_b = std::uniform_real_distribution<float>(0, 1);
  FOR_I(k) {
    const int j = RND_UNIF_INT(data->size(), *rnd);
    float v = std::exp(dist_a(*rnd));
    if (dist_b(*rnd) < 0.5) {
      v = -v;
    }
    (*data)[j] += v;
  }
}

void MutationRandomAdd(const float rate, const float scale,
                       std::vector<float> *data, std::mt19937 *rnd) {
  const int k = std::binomial_distribution<int>(data->size(), rate)(*rnd);
  auto noise_distribution =
      std::uniform_real_distribution<float>(-scale, scale);
  FOR_I(k) {
    const int j = RND_UNIF_INT(data->size(), *rnd);
    (*data)[j] += noise_distribution(*rnd);
  }
}

void MutationRandomAddAll(const float scale, std::vector<float> *data,
                          std::mt19937 *rnd) {
  auto noise_distribution =
      std::uniform_real_distribution<float>(-scale, scale);
  FOR_I(data->size()) { (*data)[i] += noise_distribution(*rnd); }
}

Genome CrossoverOnePoint(const Genome &a, const Genome &b, std::mt19937 *rnd) {
  Genome child = a;
  const int point = RND_UNIF_INT(a.weight_bank.size(), *rnd);
  std::copy(b.weight_bank.begin() + point, b.weight_bank.end(),
            child.weight_bank.begin() + point);
  return child;
}

Genome CrossoverTwoPoint(const Genome &a, const Genome &b, std::mt19937 *rnd) {
  Genome child = a;
  int p1 = RND_UNIF_INT(a.weight_bank.size(), *rnd);
  int p2 = RND_UNIF_INT(a.weight_bank.size(), *rnd);
  if (p1 > p2) {
    std::swap(p1, p2);
  }
  std::copy(b.weight_bank.begin() + p1, b.weight_bank.begin() + p2,
            child.weight_bank.begin() + p1);
  return child;
}

Genome CrossoverRandom(const Genome &a, const Genome &b, const float balance,
                       std::mt19937 *rnd) {
  Genome child = a;
  auto balance_distribution = std::uniform_real_distribution<float>(0, 1);
  FOR_I(a.weight_bank.size()) {
    if (balance_distribution(*rnd) < balance) {
      child.weight_bank[i] = b.weight_bank[i];
    }
  }
  return child;
}

const Genome &SelectionTournament(const std::vector<Genome> &population,
                                  const int k, std::mt19937 *rnd) {
  assert(!population.empty());
  int best_individual_idx = 0;
  float best_fitness = -std::numeric_limits<float>::infinity();
  FOR_I(k) {
    const int individual_idx = RND_UNIF_INT(population.size(), *rnd);
    if (population[individual_idx].fitness >= best_fitness) {
      best_fitness = population[individual_idx].fitness;
      best_individual_idx = individual_idx;
    }
  }
  return population[best_individual_idx];
}

GenomeManager::GenomeManager(const MapDef &map_definition,
                             const Options &options)
    : map_definition_(map_definition), options_(options) {

  if (options.hidden_layers.empty()) {
    weight_address_1 = weigh_allocator.CreateAddress(
        map_definition_.shape.Size() * map_definition_.num_values, Numdir());
    weight_address_2 = weigh_allocator.CreateAddress(
        Numdir() * GeneticController::HISTORY_LENGTH, Numdir());
  } else {
    int n = options.hidden_layers.front();
    weight_address_1 = weigh_allocator.CreateAddress(
        map_definition_.shape.Size() * map_definition_.num_values, n);
    weight_address_2 = weigh_allocator.CreateAddress(
        Numdir() * GeneticController::HISTORY_LENGTH, n);

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

void GenomeManager::Mutate(Genome *genome) {
  switch (options_.mutate) {
  case Options::eMutate::kAll:
    MutationRandomAddAll(options_.mutate_scale, &genome->weight_bank, &rnd_);
    break;
  case Options::eMutate::kSome: {
    MutationRandomAdd(options_.mutate_ratio, options_.mutate_scale,
                      &genome->weight_bank, &rnd_);
  } break;
  default:
    CHECK(false);
    break;
  }
}

Genome GenomeManager::Crossover(const Genome &a, const Genome &b) {
  // Cross-over
  switch (options_.cross_over) {
  case Options::eCrossOver::kOnePoint:
    if (RND_UNIF_FLOAT(rnd_) < options_.cross_over_rate) {
      return CrossoverOnePoint(a, b, &rnd_);
    } else {
      return a;
    }
  case Options::eCrossOver::kTwoPoints:
    if (RND_UNIF_FLOAT(rnd_) < options_.cross_over_rate) {
      return CrossoverTwoPoint(a, b, &rnd_);
    } else {
      return a;
    }
  case Options::eCrossOver::kNone:
    return a;

  case Options::eCrossOver::kRandom:
    if (RND_UNIF_FLOAT(rnd_) < options_.cross_over_rate) {
      return CrossoverRandom(a, b, options_.cross_over_random_balance, &rnd_);
    } else {
      return a;
    }
  }

  CHECK(false);
}

Genome GenomeManager::Random() {
  Genome child;
  child.map_definition = map_definition_;

  weigh_allocator.AllocateWeightBank(&child.weight_bank);
  neural_net::InitWeights(&child.weight_bank, &rnd_);
  return child;
}

const Genome &
GenomeManager::SelectIndividual(const std::vector<Genome> &population) {
  return SelectionTournament(
      population, population.size() * options_.tournament_ratio, &rnd_);
}

GeneticController::GeneticController(const Genome &genome,
                                     const GenomeManager *genome_manager)
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

Output GeneticController::Step(const Input &input) {
  neural_net::ForwardOneHot<neural_net::Identity>(
      input.surouding.values, genome_.map_definition.num_values,
      genome_manager_->weight_address_1(genome_.weight_bank),
      &cache_hidden_[0]);

  neural_net::ForwardOneHot<neural_net::Identity>(
      last_dirs_, Numdir(),
      genome_manager_->weight_address_2(genome_.weight_bank),
      &cache_hidden_[1]);

  neural_net::AddTo<neural_net::Identity>(cache_hidden_[0], &cache_hidden_[1]);

  if (!genome_manager_->options_.hidden_layers.empty()) {
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
  }

  int dir = neural_net::SoftMaxSampling(cache_hidden_.back(), &rnd_);

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

} // namespace genetic
} // namespace exploratron