#ifndef EXPLORATRON_CORE_ABSTRACT_ARENA_H_
#define EXPLORATRON_CORE_ABSTRACT_ARENA_H_

#include <memory>
#include <string>
#include <vector>

#include "exploratron/core/utils/maths.h"
#include "exploratron/core/utils/register.h"

namespace exploratron {

typedef std::vector<float> Scores;

class AbstractArena {
public:
  virtual ~AbstractArena() = default;
  virtual std::string Info() const { return ""; };
  virtual bool Step() = 0;
  virtual void Draw() const = 0;
  virtual Scores FinalScore() const = 0;

private:
};

struct MapDef {
  MapDef() {}
  MapDef(MatrixShape cshape, uint8_t cnum_values)
      : shape(cshape), num_values(cnum_values) {}

  MatrixShape shape;

  // Map elements are in [0, num_values).
  uint8_t num_values;
};

class AbstractControllerBuilder;

class AbstractArenaBuilder {
public:
  virtual ~AbstractArenaBuilder() = default;

  virtual std::unique_ptr<AbstractArena> Create(
      const std::vector<const AbstractControllerBuilder *> &controller_builders)
      const = 0;

  virtual std::string name() const = 0;
  virtual MapDef MapDefinition() const = 0;
  void SetParameter(std::string parameter) { parameter_ = parameter; }

protected:
  std::string parameter_;
};

REGISTRATION_CREATE_POOL(AbstractArenaBuilder);

#define REGISTER_AbstractArenaBuilder(clas, name, key)                         \
  REGISTRATION_REGISTER_CLASS(clas, name, key, AbstractArenaBuilder);

} // namespace exploratron

#endif