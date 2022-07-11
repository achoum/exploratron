#ifndef EXPLORATRON_CORE_UTILS_NEURAL_NET_H_
#define EXPLORATRON_CORE_UTILS_NEURAL_NET_H_

#include <ostream>
#include <random>
#include <sstream>
#include <stdint.h>
#include <vector>

#include "absl/types/span.h"
#include "exploratron/core/utils/logging.h"
#include "exploratron/core/utils/macros.h"

namespace exploratron {
namespace neural_net {

typedef std::vector<float> VectorF;
typedef std::vector<int> VectorI;
typedef absl::Span<float> SVectorF;
typedef absl::Span<const float> CSVectorF;

struct WeightsAddress {
  SVectorF operator()(VectorF &weights) const;

  int begin;
  int size;
};

struct WeightsAllocator {
  WeightsAddress CreateAddress(int size);
  WeightsAddress CreateAddress(int input_dim, int output_dim);
  void AllocateWeightBank(VectorF *bank);
  int size() const { return next_begin; }

  int next_begin = 0;
};

typedef float (*ActivationFn)(float);

inline float Identity(float x) { return x; }
inline float Relu(float x) { return std::max(x, 0.f); }

int NumWeights(int input_dim, int output_dim);

enum class eEnumWeights {
  SYMETRICAL_UNIFORM,
  POSITIVE_UNIFORM,
  UNIT,
};
void InitWeights(VectorF *output, std::mt19937 *rnd, const float scale = 0.1f,
                 const eEnumWeights type = eEnumWeights::SYMETRICAL_UNIFORM);

int ArgMax(const VectorF &input);

int SoftMaxSampling(const VectorF &input, std::mt19937 *rnd);

template <ActivationFn Activation, typename V>
void ForwardOneHot(const std::vector<V> &input, int num_input_values,
                   const SVectorF &weights, VectorF *output) {
  DCHECK_EQ((input.size() * num_input_values + 1) * output->size(),
            weights.size());

  // From minor to major.
  // input_value, input dim, constant, output dim

  const int i_s = input.size();
  const int o_s = output->size();
  const int offset_factor = i_s * num_input_values + 1;

  for (int o_idx = 0; o_idx < o_s; o_idx++) {
    const int w_offset = o_idx * offset_factor;
    float acc = weights[w_offset + i_s * num_input_values];
    for (int i_idx = 0; i_idx < i_s; i_idx++) {
      DCHECK_LT(input[i_idx], num_input_values);
      DCHECK_GE(input[i_idx], 0);
      const int w_idx = i_idx * num_input_values + input[i_idx];
      DCHECK_LT(w_idx, offset_factor);
      acc += weights[w_offset + w_idx];
    }
    (*output)[o_idx] = Activation(acc);
  }
}

void SetWeightOneHot(const int input_size, int num_input_values,
                     const int output_size, const int input,
                     const int input_value, const int output, const float value,
                     SVectorF *weights);

void SetWeightOneHotConstant(const int input_size, int num_input_values,
                             const int output_size, const int output,
                             const float value, SVectorF *weights);

template <ActivationFn Activation>
inline void Forward(const VectorF &input, const SVectorF &weights,
                    VectorF *output) {
  DCHECK_EQ((input.size() + 1) * output->size(), weights.size());
  const int i_s = input.size();
  const int o_s = output->size();
  const int offset_factor = i_s + 1;

  for (int o_idx = 0; o_idx < o_s; o_idx++) {
    const int w_offset = o_idx * offset_factor;
    float acc = weights[w_offset + i_s];
    for (int i_idx = 0; i_idx < i_s; i_idx++) {
      acc += weights[w_offset + i_idx] * input[i_idx];
    }
    (*output)[o_idx] = Activation(acc);
  }
}

template <ActivationFn Activation>
inline void AddTo(const VectorF &input, VectorF *output) {
  DCHECK_EQ(input.size(), output->size());
  FOR_I(input.size()) { (*output)[i] = Activation((*output)[i] + input[i]); }
}

void BoundValues(float r, VectorF *output);

} // namespace neural_net
} // namespace exploratron

#endif
