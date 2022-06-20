#ifndef EXPLORATRON_CORE_ABSTRACT_CONTROLLER_H_
#define EXPLORATRON_CORE_ABSTRACT_CONTROLLER_H_

#include "exploratron/core/abstract_arena.h"
#include "exploratron/core/utils/maths.h"
#include "exploratron/core/utils/register.h"
#include <memory>
#include <string>

namespace exploratron {

enum class eAction {
  NONE,
  MOVE,
  MELLE_ATTACK,
  MAGIC,
  AI,
};

struct Input {
  MatrixU8 surouding;
};

struct Output {
  eDirection move = eDirection::NONE;
  eAction action = eAction::MOVE;
  int magic_idx = -1;
  Vector2i target;
  bool stop = false;
};

class AbstractController {
public:
  virtual ~AbstractController() = default;
  virtual std::string Info() const { return ""; };
  virtual Output Step(const Input &) = 0;
};

class AbstractControllerBuilder {
public:
  virtual ~AbstractControllerBuilder() = default;
  virtual std::unique_ptr<AbstractController>
  Create(const MapDef &map_definition) const = 0;
  virtual std::string name() const = 0;
};

REGISTRATION_CREATE_POOL(AbstractControllerBuilder, std::string_view);

#define REGISTER_AbstractControllerBuilder(clas, name, key)                    \
  REGISTRATION_REGISTER_CLASS(clas, name, key, AbstractControllerBuilder);

} // namespace exploratron

#endif
