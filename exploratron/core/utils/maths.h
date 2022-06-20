#ifndef EXPLORATRON_CORE_UTILS_VECTOR_H_
#define EXPLORATRON_CORE_UTILS_VECTOR_H_

#include "exploratron/core/utils/logging.h"
#include "exploratron/core/utils/macros.h"
#include <ostream>
#include <random>
#include <sstream>
#include <stdint.h>
#include <vector>

namespace exploratron {

enum eDirection {
  NONE = 0,
  RIGHT = 1,
  DOWN = 2,
  LEFT = 3,
  UP = 4,
  _NUM_DIRECTIONS = 5,
};

inline eDirection ReverseDirection(eDirection d) {
  switch (d) {
  case eDirection::RIGHT:
    return eDirection::LEFT;
  case eDirection::DOWN:
    return eDirection::UP;
  case eDirection::LEFT:
    return eDirection::RIGHT;
  case eDirection::UP:
    return eDirection::DOWN;
  default:
    return eDirection::NONE;
  }
}

struct RGB {
  unsigned char r, g, b;

  bool operator==(const RGB &a) { return r == a.r && g == a.g && b == a.b; }
};

struct Vector2i {
  int x;
  int y;

  Vector2i() : x(0), y(0) {}

  Vector2i(int cx, int cy) : x(cx), y(cy) {}

  Vector2i(const int d) : Vector2i((eDirection)d) {}

  Vector2i(const eDirection &d) {
    switch (d) {
    case eDirection::RIGHT:
      x = 1;
      y = 0;
      break;
    case eDirection::DOWN:
      x = 0;
      y = 1;
      break;
    case eDirection::LEFT:
      x = -1;
      y = 0;
      break;
    case eDirection::UP:
      x = 0;
      y = -1;
      break;
    default:
      x = 0;
      y = 0;
      break;
    }
  }

  bool operator==(const Vector2i &a) const { return x == a.x && y == a.y; }
  bool operator!=(const Vector2i &a) const { return x != a.x || y != a.y; }

  Vector2i operator+(const Vector2i &b) const { return {x + b.x, y + b.y}; }
  Vector2i operator-(const Vector2i &b) const { return {x - b.x, y - b.y}; }

  Vector2i &operator+=(const Vector2i &b) {
    x += b.x;
    y += b.y;
    return *this;
  }

  bool IsZero() const { return x == 0 && y == 0; }

  int Size() const { return x * y; }

  bool PointIsInRect(const Vector2i &point) const {
    return (point.x >= 0) && (point.x < x) && (point.y >= 0) && (point.y < y);
  }

  int Length2() const { return x * x + y * y; }

  int MaxLength() const { return std::max(std::abs(x), std::abs(y)); }

  eDirection MajorDir() const {
    if (std::abs(x) > std::abs(y)) {
      if (x > 0) {
        return eDirection::RIGHT;
      } else if (x < 0) {
        return eDirection::LEFT;
      } else {
        return eDirection::NONE;
      }
    } else {
      if (y > 0) {
        return eDirection::DOWN;
      } else if (y < 0) {
        return eDirection::UP;
      } else {
        return eDirection::NONE;
      }
    }
  }

  eDirection MinorDir() const {
    if (std::abs(x) > std::abs(y)) {
      if (y > 0) {
        return eDirection::DOWN;
      } else if (y < 0) {
        return eDirection::UP;
      } else {
        return eDirection::NONE;
      }
    } else {
      if (x > 0) {
        return eDirection::RIGHT;
      } else if (x < 0) {
        return eDirection::LEFT;
      } else {
        return eDirection::NONE;
      }
    }
  }

  friend std::ostream &operator<<(std::ostream &stream, const Vector2i &v) {
    stream << v.x << " " << v.y;
    return stream;
  }
};

typedef Vector2i MatrixShape;

template <typename V> struct Matrix {

  V &operator()(int x, int y) { return values[index(x, y)]; }
  const V &operator()(int x, int y) const { return values[index(x, y)]; }
  int index(int x, int y) const { return x + y * shape.x; }
  void Initialize(const MatrixShape &s) {
    shape = s;
    values.resize(shape.x * shape.y);
  }

  friend std::ostream &operator<<(std::ostream &stream, const Matrix<V> &v) {
    stream << "(" << v.shape.x << " x " << v.shape.y << ")\n";
    FOR_Y(v.shape.y) {
      FOR_X(v.shape.x) { stream << " " << (float)v(x, y); }
      stream << "\n";
    }
    return stream;
  }

  std::vector<V> values;
  MatrixShape shape;
};

template <typename C, typename T> std::string print(const T &vs) {
  std::stringstream ss;
  ss << "(" << vs.size() << ")";
  for (const auto &v : vs) {
    ss << " " << (C)v;
  }
  return ss.str();
}

typedef Matrix<uint8_t> MatrixU8;

template <typename T> void InitRandom(T *rnd) {
  std::random_device rd;
  std::seed_seq seed{rd(), static_cast<unsigned int>(time(0))};
  rnd->seed(seed);
}

} // namespace exploratron

#endif
