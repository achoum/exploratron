#include "exploratron/core/utils/neural_net.h"
#include "exploratron/core/utils/logging.h"

#include <algorithm>
#include <assert.h>
#include <cmath>
#include <numeric>

namespace exploratron {
namespace neural_net {

int NumWeights(int input_dim, int output_dim) {
  return (input_dim + 1) * output_dim;
}

void InitWeights(VectorF *output, std::mt19937 *rnd, const float scale,
                 const eEnumWeights type) {

  switch (type) {
  case eEnumWeights::SYMETRICAL_UNIFORM: {
    std::uniform_real_distribution<float> dist(-scale, scale);
    for (auto &v : *output) {
      v = dist(*rnd);
    }
  } break;

  case eEnumWeights::POSITIVE_UNIFORM: {
    std::uniform_real_distribution<float> dist(0, scale);
    for (auto &v : *output) {
      v = dist(*rnd);
    }
  } break;

  case eEnumWeights::UNIT: {
    std::uniform_int_distribution<int> dist(0, 1);
    for (auto &v : *output) {
      v = dist(*rnd);
    }
  } break;

  default:
    CHECK(false);
  }
}

SVectorF WeightsAddress::operator()(VectorF &weights) const {
  return SVectorF(&weights[begin], size);
}

WeightsAddress WeightsAllocator::CreateAddress(int size) {
  WeightsAddress a;
  a.begin = next_begin;
  a.size = size;
  next_begin += size;
  return a;
}

WeightsAddress WeightsAllocator::CreateAddress(int input_dim, int output_dim) {
  return CreateAddress(NumWeights(input_dim, output_dim));
}

void WeightsAllocator::AllocateWeightBank(VectorF *bank) {
  bank->resize(next_begin);
}

void SetWeightOneHot(const int input_size, int num_input_values,
                     const int output_size, const int input,
                     const int input_value, const int output, const float value,
                     SVectorF *weights) {
  DCHECK_EQ((input_size * num_input_values + 1) * output_size, weights->size());
  const int offset_factor = input_size * num_input_values + 1;
  const int idx =
      input_value + input * num_input_values + output * offset_factor;
  (*weights)[idx] = value;
}

void SetWeightOneHotConstant(const int input_size, int num_input_values,
                             const int output_size, const int output,
                             const float value, SVectorF *weights) {
  DCHECK_EQ((input_size * num_input_values + 1) * output_size, weights->size());
  const int offset_factor = input_size * num_input_values + 1;
  const int idx = (offset_factor - 1) + output * offset_factor;
  (*weights)[idx] = value;
}

int ArgMax(const VectorF &input) {
  // const auto it_max = std::max_element(input.begin(), input.begin());
  // return std::distance(input.begin(), it_max);
  int j = 0;
  FOR_I(input.size()) {
    if (input[i] > input[j]) {
      j = i;
    }
  }
  return j;
}

int SoftMaxSampling(const VectorF &input, std::mt19937 *rnd) {
  assert(!input.empty());
  float sum = 0;
  for (auto v : input) {
    sum += std::exp(v);
  }
  float v = std::uniform_real_distribution<float>(0, sum)(*rnd);
  for (int i = 0; i < input.size(); i++) {
    v -= std::exp(input[i]);
    if (v <= 0) {
      return i;
    }
  }
  return input.size() - 1;
}

void BoundValues(float r, VectorF *output) {
  for (auto &value : *output) {
    if (value < -r) {
      value = -r;
    } else if (value > r) {
      value = r;
    }
  }
}

} // namespace neural_net
} // namespace exploratron
