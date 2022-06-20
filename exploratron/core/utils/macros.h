#ifndef EXPLORATRON_CORE_UTILS_MACROS_H_
#define EXPLORATRON_CORE_UTILS_MACROS_H_

#include <random>

#define FOR_I(N) for (int i = 0; i < (N); i++)
#define FOR_J(N) for (int j = 0; j < (N); j++)
#define FOR_K(N) for (int k = 0; k < (N); k++)

#define FOR_X(N) for (int x = 0; x < (N); x++)
#define FOR_Y(N) for (int y = 0; y < (N); y++)

#define RND_UNIF_INT(N, RND) std::uniform_int_distribution<int>(0, N - 1)(RND)
#define RND_UNIF_FLOAT(RND) std::uniform_real_distribution<float>(0, 1)(RND)

#endif
