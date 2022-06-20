#include "exploratron/core/abstract_game_area.h"

#include <optional>
#include <sstream>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_replace.h"
#include "exploratron/core/utils/terminal.h"

namespace exploratron {
namespace abstract_game_area {

AbstractGameArena::AbstractGameArena(
    const std::vector<const AbstractControllerBuilder *> &controller_builders,
    const MapDef &map_definition) {
  DCHECK_EQ(controller_builders.size(), 1);
  controller_ = controller_builders[0]->Create(map_definition);
  AddLog("player @ is born");
}

void Map::Draw() const {
  for (int y = 0; y < size_.y; y++) {
    for (int x = 0; x < size_.x; x++) {
      const auto &c = cell({x, y});
      DisplaySymbol cell_display{' ', -10000};
      for (const auto &e : c.entities_) {
        const auto ent_display = e->Display();
        if (!ent_display.visible_) {
          continue;
        }
        if (ent_display.priotity_ > cell_display.priotity_) {
          cell_display = ent_display;
        }
      }
      terminal::DrawSymbol(x, y, cell_display.symbol_, cell_display.color);
    }
  }
}

void Map::Step(const Output &control) {
  time_++;

  last_controlled_.reset();
  auto to_process = entities_;
  for (const auto &e : to_process) {
    if (e->contolled_ && !e->removed_) {
      e->Step(control, e, this);
      last_controlled_ = e;
      ApplyPending();
    }
  }

  Output auto_control;
  auto_control.action = eAction::AI;
  for (const auto &e : to_process) {
    if (!e->contolled_ && !e->removed_) {
      e->Step(auto_control, e, this);
      ApplyPending();
    }
  }
}

std::vector<std::shared_ptr<Entity>> Map::ControlledEntities() {
  std::vector<std::shared_ptr<Entity>> ret;
  for (const auto &e : entities_) {
    auto tags = e->Tags();
    if (e->contolled_) {
      ret.push_back(e);
    }
  }
  return ret;
}

void Map::AddEntity(const Vector2i &pos, std::shared_ptr<Entity> entity) {
  DCHECK(entity);
  entity->born_step_ = time_;
  entity->id_ = next_entity_id_++;
  entity->position_ = pos;
  pending_to_add_.push_back(entity);
}

void Map::AddEntityImplem(std::shared_ptr<Entity> entity) {
  DCHECK(entity);
  entities_.push_back(entity);
  cell(entity->position()).entities_.push_back(entity);
}

void Map::RemoveEntity(std::shared_ptr<Entity> entity) {
  DCHECK(entity);
  DCHECK(!entity->removed_);
  entity->removed_ = true;
  pending_to_remove_.push_back(entity);
}

void Map::RemoveEntityImplem(std::shared_ptr<Entity> entity) {
  DCHECK(entity);
  auto it = std::find(entities_.begin(), entities_.end(), entity);
  DCHECK(it != entities_.end());
  entities_.erase(it);
  auto &c = cell(entity->position_);
  auto it2 = std::find(c.entities_.begin(), c.entities_.end(), entity);
  DCHECK(it2 != c.entities_.end());
  c.entities_.erase(it2);
}

void Map::MoveEntity(Vector2i new_pos, std::shared_ptr<Entity> entity) {
  DCHECK(entity);
  DCHECK(!entity->removed_);
  if (new_pos == entity->position_) {
    return;
  }
  pending_to_move_.push_back({new_pos, entity});
}

void Map::MoveEntityImplem(Vector2i new_pos, std::shared_ptr<Entity> entity) {
  DCHECK(entity);
  if (entity->removed()) {
    return;
  }
  auto &c = cell(entity->position_);
  auto it = std::find(c.entities_.begin(), c.entities_.end(), entity);
  DCHECK(it != c.entities_.end());
  c.entities_.erase(it);
  cell(new_pos).entities_.push_back(entity);
  entity->position_ = new_pos;
}

void Map::ApplyPending() {
  for (auto &e : pending_to_add_) {
    AddEntityImplem(std::move(e));
  }

  for (auto &e : pending_to_remove_) {
    RemoveEntityImplem(std::move(e));
  }

  for (auto &e : pending_to_move_) {
    MoveEntityImplem(e.first, std::move(e.second));
  }

  pending_to_add_.clear();
  pending_to_remove_.clear();
  pending_to_move_.clear();
}

bool Cell::HasTag(int tag) {
  // TODO: Optimize.
  for (const auto &e : entities_) {
    auto tags = e->Tags();
    if (std::find(tags.begin(), tags.end(), tag) != tags.end()) {
      return true;
    }
  }
  return false;
}

std::shared_ptr<Entity> Cell::HasEntity(int type) {
  for (const auto &e : entities_) {
    if (e->type() == type) {
      return e;
    }
  }
  return {};
}

std::vector<std::shared_ptr<Entity>> Map::ListEntitiesWithTag(int filter_tag) {
  std::vector<std::shared_ptr<Entity>> ret;
  for (const auto &e : entities_) {
    auto tags = e->Tags();
    if (std::find(tags.begin(), tags.end(), filter_tag) == tags.end()) {
      continue;
    }
    ret.push_back(e);
  }
  return ret;
}

std::vector<std::shared_ptr<Entity>>
Map::ListVisibleEntities(Vector2i pos, int filter_tag, int not_visible_tag,
                         int max_dist) {
  const auto max_dist2 = max_dist * max_dist;
  std::vector<std::pair<int, std::shared_ptr<Entity>>> entities;
  for (const auto &e : entities_) {
    const auto dist2 = (e->position() - pos).Length2();
    if (dist2 > max_dist2) {
      continue;
    }
    auto tags = e->Tags();
    if (std::find(tags.begin(), tags.end(), filter_tag) == tags.end()) {
      continue;
    }
    if (!LineWithoutTag(pos, e->position_, not_visible_tag)) {
      continue;
    }
    entities.push_back({dist2, e});
  }
  std::sort(
      entities.begin(), entities.end(),
      [](const auto &a, const auto &b) -> bool { return a.first < b.first; });

  std::vector<std::shared_ptr<Entity>> ret;
  ret.reserve(entities.size());
  for (auto &e : entities) {
    ret.push_back(e.second);
  }
  return ret;
}

std::vector<std::shared_ptr<Entity>>
Map::ListVisibleEntitiesByType(Vector2i pos, int entity_type,
                               int not_visible_tag, int max_dist) {
  const auto max_dist2 = max_dist * max_dist;
  std::vector<std::pair<int, std::shared_ptr<Entity>>> entities;
  for (const auto &e : entities_) {
    const auto dist2 = (e->position() - pos).Length2();
    if (dist2 > max_dist2) {
      continue;
    }
    if (e->type() != entity_type) {
      continue;
    }
    if (!LineWithoutTag(pos, e->position_, not_visible_tag)) {
      continue;
    }
    entities.push_back({dist2, e});
  }
  std::sort(
      entities.begin(), entities.end(),
      [](const auto &a, const auto &b) -> bool { return a.first < b.first; });

  std::vector<std::shared_ptr<Entity>> ret;
  ret.reserve(entities.size());
  for (auto &e : entities) {
    ret.push_back(e.second);
  }
  return ret;
}

bool Map::LineWithoutTag(Vector2i p1, Vector2i p2, int tag) {
  bool good = true;
  IterateLine(p1, p2, [&](const Vector2i &p) {
    auto &c = cell(p);
    if (c.HasTag(tag)) {
      good = false;
      return false;
    }
    return true;
  });
  return good;
}

void Map::IterateLine(Vector2i p1, Vector2i p2,
                      const std::function<bool(const Vector2i &p)> &callback) {
  auto diff = p2 - p1;

  int dx = std::abs(diff.x);
  int dy = -std::abs(diff.y);

  int sx = (p1.x < p2.x) ? 1 : -1;
  int sy = (p1.y < p2.y) ? 1 : -1;
  int error = dx + dy;

  while (true) {
    if (!callback(p1)) {
      break;
    }

    if (p1 == p2) {
      break;
    }

    int e2 = 2 * error;

    bool hit_c1 = false;

    if (dx >= -dy) {
      if (e2 <= dx) {
        if (p1.y == p2.y) {
          break;
        }
        error = error + dx;
        p1.y += sy;
        hit_c1 = true;
      }

      if (e2 >= dy) {
        if (hit_c1) {
          if (!callback(p1)) {
            break;
          }
        }
        if (p1.x == p2.x) {
          break;
        }
        error = error + dy;
        p1.x += sx;
      }

    } else {
      if (e2 >= dy) {
        if (p1.x == p2.x) {
          break;
        }
        error = error + dy;
        p1.x += sx;
        hit_c1 = true;
      }

      if (e2 <= dx) {
        if (hit_c1) {
          if (!callback(p1)) {
            break;
          }
        }
        if (p1.y == p2.y) {
          break;
        }
        error = error + dx;
        p1.y += sy;
      }
    }
  }
}

void AbstractGameArena::Draw() const {
  // World
  DCHECK(map_);
  map_->Draw();

  int ui_x = map_->size_.x + 1;
  int ui_y = 0;
  int offset_content = 0;

  // Stats
  terminal::DrawTitleBar(ui_x, ui_y++, 20, "Stats", terminal::eColor::BLUE);
  const auto contolled = map_->ControlledEntities();
  DCHECK_LE(contolled.size(), 1);

  terminal::DrawString(ui_x + offset_content, ui_y++,
                       absl::StrCat("time:", map_->time()));
  if (!contolled.empty()) {
    terminal::DrawString(
        ui_x + offset_content, ui_y,
        absl::StrReplaceAll(contolled.front()->status(), {{"  ", "\n"}}));
    ui_y += 2;
  } else {
    terminal::DrawString(ui_x + offset_content, ui_y++, "You are dead",
                         terminal::eColor::RED);
    terminal::DrawString(ui_x + offset_content, ui_y++, "Press q to quit",
                         terminal::eColor::RED);
  }

  // Console.
  ui_y++;
  const int to_display = 8;
  terminal::DrawTitleBar(ui_x, ui_y++, 20, "Logs", terminal::eColor::BLUE);
  for (int i = 0; i < to_display; i++) {
    int idx = logs_.size() + i - to_display;
    if (idx >= 0) {
      terminal::DrawString(ui_x + offset_content, ui_y++, logs_[idx]);
    } else {
      ui_y++;
    }
  }

  // Keys.
  ui_y++;
  terminal::DrawTitleBar(ui_x, ui_y++, 20, "Keys", terminal::eColor::BLUE);
  terminal::DrawString(ui_x + offset_content, ui_y++, "arrows: move");
  terminal::DrawString(ui_x + offset_content, ui_y++, "a: action");
  terminal::DrawString(ui_x + offset_content, ui_y++, "space: wait");
  terminal::DrawString(ui_x + offset_content, ui_y, "h",
                       terminal::eColor::YELLOW);
  terminal::DrawString(ui_x + offset_content + 1, ui_y++, ": help");
  terminal::DrawString(ui_x + offset_content, ui_y++, "q: quit");
}

std::optional<Vector2i> Map::RandomNonOccupiedCell(std::mt19937_64 *rnd) const {
  std::vector<Vector2i> candidates;
  for (int y = 0; y < size_.y; y++) {
    for (int x = 0; x < size_.x; x++) {
      Vector2i p{x, y};
      const auto &c = cell(p);
      if (c.entities_.empty()) {
        candidates.push_back(p);
      }
    }
  }
  if (candidates.empty()) {
    return {};
  }
  const int idx =
      std::uniform_int_distribution<int>(0, candidates.size() - 1)(*rnd);
  return candidates[idx];
}

Map::Map(AbstractGameArena *parent, Vector2i size)
    : size_(size), next_entity_id_(0), parent_(parent) {
  cells_.resize(size.Size());
}

void AbstractGameArena::AddEntity(const Vector2i &pos,
                                  std::shared_ptr<Entity> entity) {
  map_->AddEntity(pos, entity);
}

bool AbstractGameArena::Step() {
  const auto control = controller_->Step({});
  map_->Step(control);
  return true;
}

void AbstractGameArena::Initialize(Vector2i size) {
  map_ = std::make_unique<Map>(this, size);
}

void Entity::SetHp(int value, std::shared_ptr<Entity> me, Map *map) {
  if (hp_ <= 0) {
    DCHECK(removed_);
  }

  hp_ = value;
  if (hp_ <= 0) {
    map->RemoveEntity(me);
  }
};

bool Entity::Hurt(int amount, std::shared_ptr<Entity> emiter,
                  std::shared_ptr<Entity> me, Map *map) {
  DCHECK(this == me.get());
  DCHECK(emiter.get() != me.get());
  if (hp() <= 0) {
    DCHECK(removed_);
    return false;
  }

  SetHp(hp() - amount, me, map);
  const bool died = hp() <= 0;

  if (died) {
    if (emiter) {
      map->AddLog(absl::StrCat(emiter->Name(), " destroys ", me->Name()));
    } else {
      map->AddLog(absl::StrCat(me->Name(), " was destroyed"));
    }
  } else {
    if (emiter) {
      map->AddLog(absl::StrCat(emiter->Name(), " hits ", me->Name(), " for ",
                               amount, " dmg. ", me->hp(), " hp left"));
    } else {
      map->AddLog(absl::StrCat(me->Name(), " was hit for ", amount, " dmg. ",
                               me->hp(), " hp left"));
    }
  }

  return died;
}

void Map::AddLog(std::string log) { parent_->AddLog(log); }

bool Entity::HasTag(int tag) const {
  auto tags = Tags();
  return std::find(tags.begin(), tags.end(), tag) != tags.end();
}

Output Entity::RandomDirection(int blocking_tag, Map *map) {
  Output action;
  int dir_idx = std::uniform_int_distribution<int>(0, 8)(map->rnd());
  if (dir_idx > 10) {
    dir_idx = 0;
  }
  action.move = (eDirection)dir_idx;
  action.action = eAction::MOVE;
  return action;
}

Output Entity::GoToDirect(int blocking_tag, Vector2i dst, Map *map,
                          bool melle_attack) {
  Output action;
  Vector2i dir = dst - position();
  auto major_dir = dir.MajorDir();
  auto minor_dir = dir.MinorDir();

  if (melle_attack && dir.Length2() == 1) {
    action.move = major_dir;
    action.action = eAction::MELLE_ATTACK;
  } else {
    if (!map->cell(position() + major_dir).HasTag(blocking_tag)) {
      action.move = major_dir;
    } else if (!map->cell(position() + minor_dir).HasTag(blocking_tag)) {
      action.move = minor_dir;
    } else {
      action.move = eDirection::NONE;
    }
    action.action = eAction::MOVE;
  }
  return action;
}

std::optional<Output> Entity::Patrol(int blocking_tag, int not_visible_tag,
                                     int patrol_type, Map *map) {
  Output output;
  output.action = eAction::MOVE;

  if (map->cell(position()).HasEntity(patrol_type)) {
    // Follow patrol
    std::vector<eDirection> candidates;
    bool has_same_as_last = false;
    auto prevent = ReverseDirection(last_patrol_dir_);
    for (int i = 1; i < eDirection::_NUM_DIRECTIONS; i++) {
      auto target_pos = position() + Vector2i((eDirection)i);
      auto &c = map->cell(target_pos);

      if (c.HasEntity(patrol_type) && !c.HasTag(blocking_tag)) {
        if (i == prevent) {
          has_same_as_last = true;
        } else {
          candidates.push_back((eDirection)i);
        }
      }
    }

    if (candidates.empty()) {
      if (has_same_as_last) {
        output.move = prevent;
        last_patrol_dir_ = output.move;
        return output;
      } else {
        return {};
      }
    } else if (candidates.size() == 1) {
      output.move = candidates.front();
      last_patrol_dir_ = output.move;
      return output;
    } else {
      output.move = candidates[std::uniform_int_distribution<int>(
          0, candidates.size() - 1)(map->rnd())];
      last_patrol_dir_ = output.move;
      return output;
    }

  } else {
    // Find patrol
    auto start_patrols = map->ListVisibleEntitiesByType(position(), patrol_type,
                                                        not_visible_tag, 4);
    if (start_patrols.empty()) {
      return {};
    }
    return GoToDirect(blocking_tag, start_patrols.front()->position(), map,
                      false);
  }
}

void Map::Explode(Vector2i pos, int radius,
                  std::function<bool(const Vector2i &)> explore) {
  // TODO: Optimize.
  // TODO: Blocked by some walls.

  // 0: Non visited.
  // 1: Visited and passed.
  // 2: Visited and blocked.
  std::vector<uint8_t> visited(NumCells(), 0);

  const auto radius2 = radius * radius;
  Vector2i p;
  for (p.y = 0; p.y < size_.y; p.y++) {
    for (p.x = 0; p.x < size_.x; p.x++) {
      if ((p - pos).Length2() <= radius2) {

        IterateLine(pos, p, [&](const Vector2i &cp) {
          auto &visited_item = visited[CellIdx(cp)];
          if (visited_item == 2) {
            return false;
          }
          if (explore(cp)) {
            // Continue
            visited_item = 1;
            return true;
          } else {
            // Bloked.
            visited_item = 2;
            return false;
          }
        });
      }
    }
  }
}

} // namespace abstract_game_area
} // namespace exploratron
