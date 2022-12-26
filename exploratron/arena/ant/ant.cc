#include "exploratron/arena/ant/ant.h"

#include <stdio.h>

#include <random>

#include "absl/strings/str_cat.h"
#include "exploratron/arena/common_game/common_game.h"
#include "exploratron/core/abstract_arena.h"
#include "exploratron/core/abstract_game_area.h"
#include "exploratron/core/utils/logging.h"
#include "exploratron/core/utils/terminal.h"

namespace exploratron {
namespace ant_area {

namespace {
// TODO
MapDef BuildMapDefinition() { return MapDef(MatrixShape(40, 20), -1); }
}  // namespace

MapDef AntArenaBuilder::MapDefinition() const { return BuildMapDefinition(); }

AntArena::AntArena(
    const std::vector<const AbstractControllerBuilder *> &controller_builders,
    std::string_view path)
    : AbstractGameArena(controller_builders, BuildMapDefinition()) {
  // common_game::InitializeFromPng("assets/map/ant_1.png",  this);
  common_game::InitializeFromTmx(absl::StrCat("exploratron/assets/map/", path),
                                 this);

  /*
    map_->IterateLine( {15, 10},{5, 5}, [&](const Vector2i &p) -> bool {
      AddLog(absl::StrCat("Add water at ", p.x, " ", p.y));
      AddEntity(p, std::make_shared<common_game::Water>());
      return true;
    });
    */
}

bool AntArena::Step() {
  if (!AbstractGameArena::Step()) {
    return false;
  }
  const auto contolled = map_->ControlledEntities();
  for (auto &c : contolled) {
    if (map()
            .cell(c->position())
            .HasEntity(common_game::EntityType::EXIT_DOOR)) {
      map().AddLog("You reached the exit");
      return false;
    }
  }
  return true;
}

}  // namespace ant_area
}  // namespace exploratron
