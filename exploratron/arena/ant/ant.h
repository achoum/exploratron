#ifndef EXPLORATRON_AREA_ANT_H_
#define EXPLORATRON_AREA_ANT_H_

#include <memory>
#include <random>
#include <vector>

#include "exploratron/arena/common_game/common_game.h"
#include "exploratron/core/abstract_arena.h"
#include "exploratron/core/abstract_controller.h"
#include "exploratron/core/abstract_game_area.h"

namespace exploratron {
namespace ant_area {

using Entity = abstract_game_area::Entity;
using DisplaySymbol = abstract_game_area::DisplaySymbol;
using AbstractGameArena = abstract_game_area::AbstractGameArena;

class AntArena : public AbstractGameArena {
public:
  AntArena(
      const std::vector<const AbstractControllerBuilder *> &controller_builders,
      std::string_view path);
  bool Step() override;
  virtual ~AntArena() = default;

private:
};

class AntArenaBuilder : public AbstractArenaBuilder {
public:
  virtual ~AntArenaBuilder() = default;

  std::unique_ptr<AbstractArena> Create(
      const std::vector<const AbstractControllerBuilder *> &controller_builders)
      const override {
    return std::make_unique<AntArena>(controller_builders, parameter_);
  }

  MapDef MapDefinition() const override;

  std::string name() const override { return "AntArenaBuilder"; }

private:
};

} // namespace ant_area

REGISTER_AbstractArenaBuilder(ant_area::AntArenaBuilder, AntArenaBuilder,
                              "Ant");

} // namespace exploratron
#endif