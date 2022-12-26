#include "exploratron/arena/common_game/common_game.h"

#include <stdio.h>

#include <queue>
#include <random>
#include <unordered_set>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/str_split.h"
#include "exploratron/core/abstract_arena.h"
#include "exploratron/core/abstract_game_area.h"
#include "exploratron/core/utils/logging.h"
#include "exploratron/core/utils/terminal.h"
#include "third_party/lodepng/lodepng.h"
#include "third_party/tinyxml2/tinyxml2.h"

namespace exploratron {
namespace common_game {

void InitializeFromPng(std::string_view path, AbstractGameArena *arena) {
  const auto builder = [&](Vector2i pos, RGB color) {
    if (color == RGB{0, 0, 0}) {
      arena->AddEntity(pos, std::make_shared<common_game::Wall>());
    } else if (color == RGB{255, 127, 39}) {
      arena->AddEntity(pos, std::make_shared<common_game::Ant>());
    } else if (color == RGB{0, 255, 0}) {
      auto player = std::make_shared<common_game::Player>();
      player->SetControlled(true);
      arena->AddEntity(pos, std::move(player));
    } else if (color == RGB{0, 0, 255}) {
      // AddEntity(pos, std::make_shared<common_game::Water>());
    } else if (color == RGB{255, 174, 201}) {
      arena->AddEntity(pos, std::make_shared<common_game::PatrolRoute>());
    } else if (color == RGB{34, 177, 76}) {
      arena->AddEntity(pos, std::make_shared<common_game::Fungus>());
    } else if (color == RGB{185, 122, 87}) {
      arena->AddEntity(pos, std::make_shared<common_game::SoftWall>());
    } else if (color == RGB{136, 0, 21}) {
      arena->AddEntity(pos, std::make_shared<common_game::Food>());
    } else if (color == RGB{163, 73, 164}) {
      arena->AddEntity(pos, std::make_shared<common_game::ExitDoor>());
    } else if (color == RGB{112, 146, 190}) {
      arena->AddEntity(pos, std::make_shared<common_game::UnbreakableWall>());
    } else if (color == RGB{127, 127, 127}) {
      arena->AddEntity(pos, std::make_shared<common_game::Boulder>());
    } else if (color == RGB{255, 0, 0}) {
      arena->AddEntity(pos, std::make_shared<common_game::AntQueen>());
    }
  };
  InitializeFromPng(path, builder, arena);
}

void InitializeFromPng(std::string_view path,
                       std::function<void(Vector2i pos, RGB color)> builder,
                       AbstractGameArena *arena) {
  // Load image.
  std::vector<unsigned char> image;
  unsigned int width, height;
  unsigned int error = lodepng::decode(image, width, height, std::string(path));
  if (error) {
    LOG(FATAL) << "decoder error " << error << " when reading " << path << " : "
               << lodepng_error_text(error);
  }

  arena->Initialize({(int)width, (int)height});
  unsigned char *iter = image.data();
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      RGB color{iter[0], iter[1], iter[2]};
      builder({x, y}, color);
      iter += 4;
    }
  }
  arena->map().ApplyPending();
}

void InitializeFromTmx(std::string_view path, AbstractGameArena *arena) {
  const auto builder = [&](Vector2i pos, int symbol,
                           const std::string &message) {
    switch (symbol) {
      case 0:
      case ' ':
        break;
      case 219:
        arena->AddEntity(pos, std::make_shared<common_game::Wall>());
        break;
      case 'a':
        arena->AddEntity(pos, std::make_shared<common_game::Ant>());
        break;
      case 2: {
        auto player = std::make_shared<common_game::Player>();
        player->SetControlled(true);
        arena->AddEntity(pos, std::move(player));
      } break;
      case ',':
        arena->AddEntity(pos, std::make_shared<common_game::PatrolRoute>());
        break;
      case '"':
        arena->AddEntity(pos, std::make_shared<common_game::Fungus>());
        break;
      case '&':
        arena->AddEntity(pos, std::make_shared<common_game::SoftWall>());
        break;
      case 'f':
        arena->AddEntity(pos, std::make_shared<common_game::Food>());
        break;
      case 'x':
        arena->AddEntity(pos, std::make_shared<common_game::Explosive>());
        break;
      case '>':
        arena->AddEntity(pos, std::make_shared<common_game::ExitDoor>());
        break;
      case 'U':
        arena->AddEntity(pos, std::make_shared<common_game::UnbreakableWall>());
        break;
      case 'O':
        arena->AddEntity(pos, std::make_shared<common_game::Boulder>());
        break;
      case 'A':
        arena->AddEntity(pos, std::make_shared<common_game::AntQueen>());
        break;

      case '+':
        arena->AddEntity(pos, std::make_shared<common_game::AutomaticDoor>());
        break;
      case 255:  // button
        arena->AddEntity(pos, std::make_shared<common_game::Button>());
        break;
      case 24:  // up
        arena->AddEntity(
            pos, std::make_shared<common_game::ConveyorBelt>(eDirection::UP));
        break;
      case 25:  // down
        arena->AddEntity(
            pos, std::make_shared<common_game::ConveyorBelt>(eDirection::DOWN));
        break;
      case 26:  // right
        arena->AddEntity(pos, std::make_shared<common_game::ConveyorBelt>(
                                  eDirection::RIGHT));
        break;
      case 27:  // left
        arena->AddEntity(
            pos, std::make_shared<common_game::ConveyorBelt>(eDirection::LEFT));
        break;
      case 254:  // steel wall
        arena->AddEntity(pos, std::make_shared<common_game::SteelWall>());
        break;
      case 'X':  // explosive barel
        arena->AddEntity(pos, std::make_shared<common_game::ExplosiveBarel>());
        break;
      case '.':
        arena->AddEntity(pos, std::make_shared<common_game::Wire>());
        break;
      case 'p':
        arena->AddEntity(pos, std::make_shared<common_game::ProxySensor>());
        break;
      case '?':
        arena->AddEntity(pos, std::make_shared<common_game::Message>(message));
        break;
      case 't':
        arena->AddEntity(pos, std::make_shared<common_game::Turret>());
        break;
      case 'r':
        arena->AddEntity(pos, std::make_shared<common_game::Robot>());
        break;
      case 'G':
        arena->AddEntity(pos, std::make_shared<common_game::Goliat>());
        break;
      case 'W':
        arena->AddEntity(pos, std::make_shared<common_game::Worm>());
        break;

      default:
        LOG(FATAL) << "Symbol " << symbol << "(" << (char)symbol << ") unknown";
        break;
    }
  };
  InitializeFromTmx(path, builder, arena);
}

void InitializeFromTmx(
    std::string_view path,
    std::function<void(Vector2i pos, int symbol, const std::string &message)>
        builder,
    AbstractGameArena *arena) {
  tinyxml2::XMLDocument doc;
  doc.LoadFile(std::string(path).c_str());
  auto *map = doc.FirstChildElement("map");
  CHECK(map);
  auto width = map->IntAttribute("width");
  auto height = map->IntAttribute("height");

  if (width > 67) {
    LOG(INFO) << "Map's width is greater than 67";
  }
  if (height > 44) {
    LOG(INFO) << "Map's height is greater than 44";
  }

  arena->Initialize({width, height});

  LOG(INFO) << "Load map " << path << " with size " << width << " x " << height;

  {
    auto *cur_layer = map->FirstChildElement("layer");
    while (cur_layer != nullptr) {
      auto *data = cur_layer->FirstChildElement("data");
      CHECK(data);
      CHECK(std::strcmp(data->Attribute("encoding"), "csv") == 0);
      auto *cstr_csv_data = data->GetText();
      std::string_view csv_data(cstr_csv_data);
      std::vector<std::string> csv_rows = absl::StrSplit(csv_data, "\n");
      CHECK_EQ(csv_rows.size() - 2, height);
      int row_idx = 0;
      for (auto &csv_row : csv_rows) {
        if (csv_row.empty()) {
          continue;
        }
        std::vector<std::string> csv_cols = absl::StrSplit(csv_row, ",");
        // CHECK_EQ(csv_cols.size()-1, width);
        int col_idx = 0;
        for (auto &csv_col : csv_cols) {
          if (!csv_col.empty()) {
            const int symbol = std::stoi(csv_col);
            // Tiled index the symbols starting at 1.
            if (symbol > 0) {
              if (col_idx < width && row_idx < height) {
                builder({col_idx, row_idx}, symbol - 1, {});
              }
            }
          }
          col_idx++;
        }

        row_idx++;
      }
      cur_layer = cur_layer->NextSiblingElement("layer");
    }
  }

  {
    auto *cur_objectgroup = map->FirstChildElement("objectgroup");
    while (cur_objectgroup != nullptr) {
      auto *cur_object = cur_objectgroup->FirstChildElement("object");
      while (cur_object != nullptr) {
        int gid = cur_object->IntAttribute("gid");
        int x = cur_object->IntAttribute("x");
        int y = cur_object->IntAttribute("y");
        CHECK(gid >= 0);
        CHECK(x >= 0);
        CHECK(y >= 0);

        if (x < width * 16 & y < height * 16) {
          std::string message;
          auto *cur_properties = cur_object->FirstChildElement("properties");
          CHECK(cur_properties);
          auto *cur_property = cur_properties->FirstChildElement("property");
          while (cur_property != nullptr) {
            auto *raw_name = cur_property->Attribute("name");
            auto *raw_value = cur_property->Attribute("value");
            if (std::strcmp(raw_name, "message") == 0) {
              message = raw_value;
            }

            cur_property = cur_property->NextSiblingElement("property");
          }
          message = absl::StrReplaceAll(message, {{"\\n", "\n"}});

          builder({x / 16, y / 16 - 1}, gid - 1, message);
        }

        cur_object = cur_object->NextSiblingElement("object");
      }
      cur_objectgroup = cur_objectgroup->NextSiblingElement("objectgroup");
    }
  }

  arena->map().ApplyPending();
}

void Ant::Step(Output action, std::shared_ptr<Entity> me, Map *map) {
  if (action.action == eAction::AI) {
    action = StepAI(me, map);
  }
  StepExecutePlan(action, me, map);
}

Output Ant::StepAI(std::shared_ptr<Entity> me, Map *map) {
  Output action;

  auto &cell = map->cell(position());

  // Target visible ennemi
  auto visible_entities = map->ListVisibleEntities(
      me->position(), Tag::ANT_TARGET, Tag::WALL_LIKE, 20);

  std::shared_ptr<Entity> best_target;
  bool best_target_is_low_priority;

  for (auto &e : visible_entities) {
    if (e.get() == this) {
      continue;
    }
    const bool low_priority = e->HasTag(Tag::ANT_LOW_PRIORITY);
    if (!best_target) {
      best_target = e;
      best_target_is_low_priority = low_priority;
    } else if (best_target_is_low_priority && !low_priority) {
      best_target = e;
      best_target_is_low_priority = false;
      break;
    }
  }
  if (best_target) {
    // Move/attack toward target
    last_target_ = best_target->position();
    last_target_time_ = map->time();
    auto output = GoToDirect(Tag::NON_PASSABLE, best_target->position(), map,
                             !best_target->HasTag(Tag::DONT_ATTACK));

    // Drop pheromone.
    if (output.action == eAction::MOVE) {
      auto target_dir = output.move;
      auto noncast_pheromone = cell.HasEntity(EntityType::PHEROMONE);
      auto *pheromone = dynamic_cast<Pheromone *>(noncast_pheromone.get());
      if (noncast_pheromone && pheromone->dir() == target_dir) {
      } else {
        map->AddEntity(position(), std::make_shared<Pheromone>(target_dir));
      }
    }
    return output;
  }

  // Target past visible ennemi
  if (last_target_.has_value()) {
    if (last_target_.value() == position() ||
        (map->time() - last_target_time_) > 20) {
      last_target_ = {};
    } else {
      return GoToDirect(Tag::NON_PASSABLE, last_target_.value(), map, false);
    }
  }

  // Pheromone
  auto maybe_noncast_pheromone = cell.HasEntity(EntityType::PHEROMONE);
  if (maybe_noncast_pheromone) {
    auto *pheromone = dynamic_cast<Pheromone *>(maybe_noncast_pheromone.get());
    Output action;
    action.action = eAction::MOVE;
    if (!map->cell(position() + Vector2i(pheromone->dir()))
             .HasTag(Tag::NON_PASSABLE)) {
      action.move = pheromone->dir();
      last_pheromone_time_ = map->time();
      last_pheromone_dir_ = action.move;
      return action;
    }
  } else if (map->time() - last_pheromone_time_ < 5) {
    if (!map->cell(position() + Vector2i(last_pheromone_dir_))
             .HasTag(Tag::NON_PASSABLE)) {
      Output action;
      action.action = eAction::MOVE;
      action.move = last_pheromone_dir_;
      return action;
    } else {
      for (int dir = 0; dir < eDirection::_NUM_DIRECTIONS; dir++) {
        if (dir == last_pheromone_dir_) {
          continue;
        }

        auto &target_cell = map->cell(position() + Vector2i(dir));
        if (target_cell.HasEntity(EntityType::PHEROMONE) &&
            !target_cell.HasTag(Tag::NON_PASSABLE)) {
          Output action;
          action.action = eAction::MOVE;
          action.move = (eDirection)dir;
          return action;
        }
      }
    }
  }

  // Patrol
  auto maybe_patrol =
      Patrol(Tag::NON_PASSABLE, Tag::WALL_LIKE, EntityType::PATROL_ROUTE, map);
  if (maybe_patrol.has_value()) {
    return maybe_patrol.value();
  }

  // Random walk
  return RandomDirection(Tag::NON_PASSABLE, map);
}

void Ant::StepExecutePlan(Output action, std::shared_ptr<Entity> me, Map *map) {
  switch (action.action) {
    case eAction::MOVE: {
      Vector2i dir(action.move);
      Vector2i new_pos = position() + dir;
      if (!map->Contains(new_pos)) {
        break;
      }
      const auto &cell = map->cell(new_pos);

      bool passable = true;
      for (const auto &e : cell.entities_) {
        if (e->type() == EntityType::CONVEYOR_BELT &&
            dynamic_cast<ConveyorBelt *>(e.get())->direction() ==
                ReverseDirection(action.move)) {
          passable = false;
        }
        if (e->HasTag(Tag::NON_PASSABLE)) {
          passable = false;
        }
      }

      if (!passable) {
        break;
      }
      map->MoveEntity(new_pos, me);
    } break;

    case eAction::MELLE_ATTACK: {
      Vector2i dir(action.move);
      Vector2i new_pos = position() + dir;
      if (!map->Contains(new_pos)) {
        break;
      }
      for (const auto &e : map->cell(new_pos).entities_) {
        if (e->HasTag(Tag::ANT_TARGET)) {
          e->Hurt(1, me, e, map);
          break;
        }
      }
    } break;
  }
}

void AntQueen::Step(Output action, std::shared_ptr<Entity> me, Map *map) {
  if (action.action == eAction::AI) {
    action = StepAI(me, map);
  }
  StepExecutePlan(action, me, map);
}

Output AntQueen::StepAI(std::shared_ptr<Entity> me, Map *map) {
  if (time_to_spwan_ <= 0) {
    // Target visible ennemi
    auto visible_entities = map->ListVisibleEntities(
        me->position(), Tag::ANT_TARGET, Tag::WALL_LIKE, 30);

    for (auto &e : visible_entities) {
      if (e.get() == this) {
        continue;
      }

      Output action;
      action.action = eAction::MAGIC;
      action.magic_idx = CREATE_ANT;
      action.target = e->position();
      time_to_spwan_ = 20;
      return action;
    }
  } else {
    time_to_spwan_--;
  }

  // Random walk
  return RandomDirection(Tag::NON_PASSABLE, map);
}

void AntQueen::StepExecutePlan(Output action, std::shared_ptr<Entity> me,
                               Map *map) {
  switch (action.action) {
    case eAction::MOVE: {
      Vector2i dir(action.move);
      Vector2i new_pos = position() + dir;
      if (!map->cell(new_pos).HasTag(Tag::NON_PASSABLE)) {
        map->MoveEntity(new_pos, me);
      }
    } break;

    case eAction::MELLE_ATTACK: {
      Vector2i dir(action.move);
      Vector2i new_pos = position() + dir;
      for (const auto &e : map->cell(new_pos).entities_) {
        if (e->HasTag(Tag::ANT_TARGET)) {
          e->Hurt(1, me, e, map);
          break;
        }
      }
    } break;

    case eAction::MAGIC: {
      switch (action.magic_idx) {
        case eMagic::CREATE_ANT: {
          map->AddLog("ant queen lay eggs");
          for (int dir = 1; dir < eDirection::_NUM_DIRECTIONS; dir++) {
            auto target_pos = position() + Vector2i(dir);
            auto &target_cell = map->cell(target_pos);
            if (!target_cell.HasEntity(Tag::NON_PASSABLE)) {
              auto ant = std::make_shared<Ant>();
              ant->SetTarget(action.target, map);
              map->AddEntity(target_pos, ant);
            }
          }
        } break;
      }
    }
  }
}

std::vector<Action> AntQueen::AvailableMagics() const {
  std::vector<Action> magics;
  magics.push_back({CREATE_ANT,
                    absl::StrCat("create ant [", time_to_spwan_, " left]"), 'a',
                    30});
  return magics;
}

void Player::Step(Output action, std::shared_ptr<Entity> me, Map *map) {
  switch (action.action) {
    case eAction::MOVE:
      StepMove(action, me, map);
      break;
    case eAction::MAGIC:
      StepMagic(action, me, map);
      break;
  }
}

void Player::StepMove(Output action, std::shared_ptr<Entity> me, Map *map) {
  if (action.move == eDirection::NONE) {
    return;
  }
  Vector2i dir(action.move);
  Vector2i new_pos = position() + dir;
  if (!map->Contains(new_pos)) {
    return;
  }
  auto &cell = map->cell(new_pos);

  bool passable = true;
  for (const auto &e : cell.entities_) {
    if (e->type() == EntityType::CONVEYOR_BELT &&
        dynamic_cast<ConveyorBelt *>(e.get())->direction() ==
            ReverseDirection(action.move)) {
      passable = false;
    }

    if (e->HasTag(Tag::NON_PASSABLE)) {
      passable = false;
      if (e->type() == EntityType::STEEL_DOOR) {
        map->AddLog(e->Name() + " is closed");
      }
    }

    if (e->HasTag(Tag::ACTIONABLE)) {
      map->AddLog(absl::StrCat(me->Name(), " activate ", e->Name()));
      e->ReceiveSignal(SignalType::PRESS, e, map);
      return;
    }

    if (e->HasTag(Tag::PLAYER_TARGET)) {
      e->Hurt(1, me, e, map);
      return;
    }

    if (e->HasTag(Tag::PUSHABLE)) {
      Vector2i new_pos2 = new_pos + dir;
      auto &cell2 = map->cell(new_pos2);
      if (!cell2.HasTag(Tag::NON_PASSABLE)) {
        map->MoveEntity(new_pos2, e);
        map->MoveEntity(new_pos, me);
        map->AddLog(absl::StrCat(me->Name(), " push ", e->Name()));
        return;
      }
    }
  }

  if (!passable) {
    return;
  }

  for (const auto &e : cell.entities_) {
    if (e->HasTag(Tag::ITEM)) {
      auto type = e->type();
      switch (type) {
        case EntityType::FOOD:
          map->AddLog(absl::StrCat(me->Name(), " find food"));
          num_food_++;
          break;
        case EntityType::EXPLOSIVE:
          map->AddLog(absl::StrCat(me->Name(), " find explosive"));
          num_explosive_++;
          break;
        default:
          LOG(FATAL) << "Cannot pick item " << type;
          break;
      }
      map->RemoveEntity(e);
    }
  }

  map->MoveEntity(new_pos, me);
}

std::optional<Vector2i> Player::ThrowPosition(Output action,
                                              std::shared_ptr<Entity> me,
                                              Map *map) {
  std::optional<Vector2i> last_good;
  const auto process_cell = [&](const Vector2i &p) {
    auto &cell = map->cell(p);
    if (cell.HasTag(Tag::WALL_LIKE)) {
      return false;
    }
    last_good = p;
    return true;
  };
  map->IterateLine(me->position(), action.target, process_cell);
  return last_good;
}

void Player::StepMagic(Output action, std::shared_ptr<Entity> me, Map *map) {
  switch (action.magic_idx) {
    case eMagic::FIREBALL: {
      if (energy_ < 1) {
        map->AddLog("Not enought energy for fireball");
        break;
      }
      energy_ -= 1;

      Vector2i last_good;
      bool found_good;
      const auto process_cell = [&](const Vector2i &p) {
        auto &cell = map->cell(p);
        if (cell.HasTag(Tag::WALL_LIKE)) {
          return false;
        }
        found_good = true;
        last_good = p;
        for (auto &e : cell.entities_) {
          if (e->HasTag(Tag::PLAYER_TARGET)) {
            e->Hurt(1, me, e, map);
            return false;
          }
        }
        return true;
      };
      map->IterateLine(me->position(), action.target, process_cell);
      if (found_good) {
        map->AddEntity(last_good, std::make_shared<Fire>());
        map->AddLog(Name() + " throws fireball");
      }
    } break;

    case eMagic::THROW_FOOD: {
      if (num_food_ < 1) {
        map->AddLog("No food to throw");
        break;
      }
      num_food_ -= 1;

      auto target_pos = ThrowPosition(action, me, map);
      if (target_pos.has_value()) {
        map->AddLog(Name() + " throws food");
        map->AddEntity(target_pos.value(), std::make_shared<Food>());
      }
    } break;

    case eMagic::EXPLOSIVE: {
      if (num_explosive_ < 1) {
        map->AddLog("No explosive to throw");
        break;
      }
      num_explosive_ -= 1;

      auto target_pos = ThrowPosition(action, me, map);
      if (target_pos.has_value()) {
        map->AddLog(Name() + " throws explosive");
        map->AddEntity(target_pos.value(), std::make_shared<Explosive>(true));
      }
    } break;
  }
}

void Water::Step(Output action, std::shared_ptr<Entity> me, Map *map) {
  // TODO
}

void Fungus::Step(Output action, std::shared_ptr<Entity> me, Map *map) {
  int local_counter = (map->time() - born_step() + id());

  // Spread
  if (left_ > 0 && (local_counter % 2) == 0) {
    std::vector<int> orders{1, 2, 3, 4};
    std::shuffle(orders.begin(), orders.end(), map->rnd());
    for (int dir : orders) {
      auto target_pos = position() + Vector2i(dir);
      auto &target_cell = map->cell(target_pos);
      if (!target_cell.HasTag(Tag::FUNGUS_LIKE) &&
          !target_cell.HasTag(Tag::WALL_LIKE)) {
        /*if (std::uniform_real_distribution()(map->rnd()) < 0.05) {
          map->AddEntity(target_pos, std::make_shared<FungusTower>());
        } else*/
        {
          // const int remove = (left_ + 1) / 2;
          // left_ -= remove;
          map->AddEntity(target_pos, std::make_shared<Fungus>(left_ - 1));
        }
        break;
      }
    }
  }

  /*
  // Hurt mob
  if ((local_counter % 2) == 0) {
    for (const auto& e : map->cell(position()).entities_) {
      if (e->HasTag(Tag::FUNGUS_TARGET)) {
        e->Hurt(1, me, e, map);
        break;
      }
    }
  }
  */
}

void FungusTower::Step(Output action, std::shared_ptr<Entity> me, Map *map) {}

std::vector<Action> Player::AvailableMagics() const {
  std::vector<Action> magics;
  if (has_fireball_) {
    magics.push_back({eMagic::FIREBALL, "fire ball [1 mp]", 'b', 4});
  }

  if (num_food_ > 0) {
    magics.push_back({eMagic::THROW_FOOD,
                      absl::StrCat("throw food [", num_food_, " left]"), 'f',
                      5});
  }

  if (num_explosive_ > 0) {
    magics.push_back(
        {eMagic::EXPLOSIVE,
         absl::StrCat("throw explosive [", num_explosive_, " left]"), 'x', 5});
  }

  return magics;
}

void Fire::Step(Output action, std::shared_ptr<Entity> me, Map *map) {
  const int life = map->time() - born_step();

  // The fire dies.
  if (life > 20) {
    map->RemoveEntity(me);
    return;
  }

  auto &cell = map->cell(position());

  // Spread
  if (life >= 1 && cell.HasTag(Tag::FLAMABLE)) {
    for (int dir = 1; dir < eDirection::_NUM_DIRECTIONS; dir++) {
      auto target = position() + Vector2i(dir);
      auto target_cell = map->cell(target);
      if (target_cell.HasTag(Tag::FLAMABLE) &&
          !target_cell.HasEntity(EntityType::FIRE)) {
        map->AddEntity(target, std::make_shared<Fire>());
      }
    }
  }

  // Hurt entity.
  for (auto &e : cell.entities_) {
    if (!e->HasTag(Tag::FIRE_TARGET)) {
      continue;
    }
    e->Hurt(1, me, e, map);
  }
}

void Food::Step(Output action, std::shared_ptr<Entity> me, Map *map) {
  if (map->time() - last_time_eaten_ < 2) {
    amount_left--;
    if (amount_left > 0) {
      map->AddLog("Food is consumed. " + std::to_string(amount_left) + " left");
    }
  }

  if (amount_left <= 0) {
    map->AddLog("Food fully consumed");
    map->RemoveEntity(me);
    return;
  }
}

bool Food::Hurt(int amount, std::shared_ptr<Entity> emiter,
                std::shared_ptr<Entity> me, Map *map) {
  if (emiter && emiter->HasTag(Tag::ATTACK_IS_EAT_FOOD)) {
    last_time_eaten_ = map->time();
    return false;
  }
  return Entity::Hurt(amount, emiter, me, map);
}

void Pheromone::Step(Output action, std::shared_ptr<Entity> me, Map *map) {
  const int life = map->time() - born_step();
  // Dies.
  if (life > 150) {
    map->RemoveEntity(me);
    return;
  }
}

std::vector<int> Explosive::Tags() const {
  if (active_) {
    return {Tag::FIRE_TARGET, Tag::EXPLOSION_TARGET, Tag::LASER_TARGET,
            Tag::MOVED_BY_CONVEYOR_BELT, Tag::KILLED_BY_AUTOMATIC_METAL_DOOR};
  } else {
    return {
        Tag::FIRE_TARGET,
        Tag::EXPLOSION_TARGET,
        Tag::LASER_TARGET,
        Tag::ITEM,
        Tag::MOVED_BY_CONVEYOR_BELT,
        Tag::KILLED_BY_AUTOMATIC_METAL_DOOR,
    };
  }
}

void Explosive::Step(Output action, std::shared_ptr<Entity> me, Map *map) {
  if (!active_) {
    return;
  }
  left_--;
  if (left_ <= 0) {
    Explode(me, map);
  }
}

bool Explosive::Hurt(int amount, std::shared_ptr<Entity> emiter,
                     std::shared_ptr<Entity> me, Map *map) {
  DCHECK(this == me.get());
  if (removed()) {
    return false;
  }

  if (!active_) {
    active_ = true;
    left_ = 2;
  }
  return false;

  // Explode(me, map);
  // return false;
}

void Explosive::Explode(std::shared_ptr<Entity> me, Map *map) {
  map->RemoveEntity(me);
  CreateExplosion(position(), me, map, 10, 2);
}

DisplaySymbol Explosive::Display() const {
  if (active_) {
    return DisplaySymbol{terminal::eSymbol::NOTHING, 90,
                         .color = terminal::eColor::YELLOW,
                         .character = std::to_string(left_)[0]};
  } else {
    return DisplaySymbol{terminal::eSymbol::EXPLOSIVE_ITEM, 40,
                         .color = terminal::eColor::GREEN};
  }
}

void Explosion::Step(Output action, std::shared_ptr<Entity> me, Map *map) {
  map->RemoveEntity(me);
}

void CreateExplosion(const Vector2i &position, std::shared_ptr<Entity> me,
                     Map *map, int damages, int radius) {
  const auto explode = [&](Vector2i pos) -> bool {
    bool blocked = false;
    auto &cell = map->cell(pos);
    for (auto &e : cell.entities_) {
      if (e == me || e->removed()) {
        continue;
      }
      const bool can_block = e->HasTag(Tag::WALL_LIKE);
      if (!e->HasTag(Tag::EXPLOSION_TARGET)) {
        if (can_block) {
          blocked = true;
        }
        continue;
      }
      e->Hurt(damages, me, e, map);
      if (can_block && !e->removed()) {
        blocked = true;
      }
    }
    if (!blocked) {
      map->AddEntity(pos, std::make_shared<Explosion>());
    }
    return !blocked;
  };

  map->Explode(position, radius, explode);
}

void ExplosiveBarel::Explode(std::shared_ptr<Entity> me, Map *map) {
  map->RemoveEntity(me);
  CreateExplosion(position(), me, map, 10, 4);
}

void ExplosiveBarel::Step(Output action, std::shared_ptr<Entity> me, Map *map) {
  if (!active_) {
    return;
  }
  left_--;
  if (left_ <= 0) {
    Explode(me, map);
  }
}

bool ExplosiveBarel::Hurt(int amount, std::shared_ptr<Entity> emiter,
                          std::shared_ptr<Entity> me, Map *map) {
  DCHECK(this == me.get());
  if (removed()) {
    return false;
  }

  if (!active_) {
    active_ = true;
    left_ = 2;
  }
  return false;
}

DisplaySymbol ExplosiveBarel::Display() const {
  if (active_) {
    return DisplaySymbol{terminal::eSymbol::NOTHING, 90,
                         .color = terminal::eColor::YELLOW,
                         .character = std::to_string(left_)[0]};
  } else {
    return DisplaySymbol{terminal::eSymbol::BARREL, 50,
                         .color = terminal::eColor::GREEN};
  }
}

void ConveyorBelt::Step(Output action, std::shared_ptr<Entity> me, Map *map) {
  auto target_pos = position() + Vector2i(direction_);
  auto &target_cell = map->cell(target_pos);
  const bool target_is_item = target_cell.HasTag(Tag::ITEM);
  const bool target_is_actionable = target_cell.HasTag(Tag::ACTIONABLE);
  const bool target_is_non_passable = target_cell.HasTag(Tag::NON_PASSABLE);

  if (target_is_non_passable && !target_is_actionable) {
    return;
  }

  auto &cell = map->cell(position());
  for (auto &e : cell.entities_) {
    if (e->last_conveyor_move_time_ == map->time()) {
      continue;
    }
    if (!e->HasTag(Tag::MOVED_BY_CONVEYOR_BELT)) {
      continue;
    }
    if (target_is_item && e->HasTag(Tag::ITEM)) {
      continue;
    }

    if (target_is_actionable) {
      if (e->HasTag(Tag::CAN_ACTION_BUTTON_ON_CONVEYOR_BELT)) {
        for (const auto &e2 : target_cell.entities_) {
          if (e2->HasTag(Tag::ACTIONABLE)) {
            map->AddLog(absl::StrCat(e->Name(), " activate ", e2->Name()));
            e2->ReceiveSignal(SignalType::PRESS, e2, map);
            return;
          }
        }
      }
      if (target_is_non_passable) {
        continue;
      }
    }

    map->MoveEntity(target_pos, e);
    e->last_conveyor_move_time_ = map->time();
  }
}

DisplaySymbol ConveyorBelt::Display() const {
  return DisplaySymbol{
      (terminal::eSymbol)((int)terminal::eSymbol::ARROW_RIGHT + direction_ - 1),
      1, .color = terminal::eColor::GRAY};
}

void SendSignal(Vector2i pos, Map *map, int signal) {
  auto &original_cell = map->cell(pos);
  if (!original_cell.HasEntity(EntityType::WIRE)) {
    return;
  }

  std::vector<bool> visited(map->NumCells(), false);
  std::queue<Vector2i> next;
  next.push(pos);

  while (!next.empty()) {
    auto n = next.front();
    next.pop();
    for (int dir = 1; dir < eDirection::_NUM_DIRECTIONS; dir++) {
      auto target = n + Vector2i(dir);
      if (visited[map->CellIdx(target)]) {
        continue;
      }
      auto &cell = map->cell(target);
      if (!cell.HasEntity(EntityType::WIRE)) {
        continue;
      }
      visited[map->CellIdx(target)] = true;
      next.push(target);
      for (auto &e : cell.entities_) {
        if (e->HasTag(Tag::RECEIVE_ELETRIC_SIGNAL)) {
          e->ReceiveSignal(signal, e, map);
        }
      }
    }
  }
}

void Button::ReceiveSignal(int signal, std::shared_ptr<Entity> me, Map *map) {
  state ^= true;
  SendSignal(me->position(), map, SignalType::RECEIVE_ELETRICITY);
}

void AutomaticDoor::ReceiveSignal(int signal, std::shared_ptr<Entity> me,
                                  Map *map) {
  closed_ ^= 1;
  if (closed_) {
    map->AddLog("door closed");
  } else {
    map->AddLog("door open");
  }
}

void AutomaticDoor::Step(Output action, std::shared_ptr<Entity> me, Map *map) {
  if (closed_) {
    auto &cell = map->cell(position());
    for (auto &e : cell.entities_) {
      if (!e->HasTag(Tag::KILLED_BY_AUTOMATIC_METAL_DOOR)) {
        continue;
      }
      e->Hurt(10, me, e, map);
    }
  }
}

void ProxySensor::Step(Output action, std::shared_ptr<Entity> me, Map *map) {
  std::vector<int> entity_ids;

  for (int x = -1; x <= 1; x++) {
    for (int y = -1; y <= 1; y++) {
      // for (int dir = 0; dir < eDirection::_NUM_DIRECTIONS; dir++) {
      auto target_pos = position() + Vector2i(x, y);
      auto &target_cell = map->cell(target_pos);
      for (auto &e : target_cell.entities_) {
        if (e->HasTag(Tag::TRIGGER_PROXY_SENSOR)) {
          entity_ids.push_back(e->id());
        }
      }
    }
  }

  std::sort(entity_ids.begin(), entity_ids.end());

  // Set difference.

  std::vector<int> difference(entity_ids.size());
  auto it = std::set_difference(entity_ids.begin(), entity_ids.end(),
                                last_entity_ids_.begin(),
                                last_entity_ids_.end(), difference.begin());
  difference.erase(it, difference.end());

  if (!difference.empty()) {
    map->AddLog("proxy sensor triggered");
    SendSignal(me->position(), map, SignalType::RECEIVE_ELETRICITY);
  }
  last_entity_ids_ = entity_ids;
}

void Message::Step(Output action, std::shared_ptr<Entity> me, Map *map) {
  auto &cell = map->cell(position());
  if (cell.HasEntity(EntityType::PLAYER)) {
    map->AddLog(message_);
  }
}

bool Laser::TestPos(const Vector2i &pos, std::shared_ptr<Entity> me, Map *map) {
  if (!map->Contains(pos)) {
    return false;
  }
  const auto &cell = map->cell(pos);

  bool stopped = false;
  for (const auto &e : cell.entities_) {
    if (e->HasTag(Tag::LASER_TARGET)) {
      e->Hurt(2, me, e, map);
      stopped = true;
    }
    if (e->HasTag(Tag::NON_PASSABLE)) {
      stopped = true;
    }
  }

  if (stopped) {
    return false;
  } else {
    return true;
  }
}

void Laser::Step(Output action, std::shared_ptr<Entity> me, Map *map) {
  if (!TestPos(position(), me, map)) {
    map->RemoveEntity(me);
    return;
  }
  Vector2i dir(dir_);
  Vector2i new_pos = position() + dir;
  if (TestPos(new_pos, me, map)) {
    map->MoveEntity(new_pos, me);
  } else {
    map->RemoveEntity(me);
  }
}

DisplaySymbol Laser::Display() const {
  return DisplaySymbol{(dir_ == 1 || dir_ == 3)
                           ? terminal::eSymbol::LASER_LEFT_RIGHT
                           : terminal::eSymbol::LASER_UP_DOWN,
                       30, .color = terminal::eColor::YELLOW};
}

void Turret::Step(Output action, std::shared_ptr<Entity> me, Map *map) {
  for (int dir_idx = 1; dir_idx < eDirection::_NUM_DIRECTIONS; dir_idx++) {
    Vector2i dir(dir_idx);
    Vector2i cur = position();
    bool attack = false;

    // Scan direction
    while (true) {
      cur += dir;
      if (!map->Contains(cur)) {
        break;
      }
      const auto &cell = map->cell(cur);
      bool stopped = false;
      for (const auto &e : cell.entities_) {
        if (e->HasTag(Tag::ROBOT_TARGET)) {
          attack = true;
          break;
        }
        if (e->HasTag(Tag::NON_PASSABLE)) {
          stopped = true;
          break;
        }
      }
      if (stopped || attack) {
        break;
      }
    }

    if (attack) {
      attack_left_ = 2;
      attack_dir = dir_idx;
    }
  }

  if (attack_left_ > 0) {
    attack_left_--;

    auto laser = std::make_shared<Laser>(attack_dir);
    auto laser_pos = position() + Vector2i(attack_dir);
    if (laser->TestPos(laser_pos, me, map)) {
      map->AddEntity(laser_pos, laser);
    } else {
    }
  }
}

void Robot::Step(Output action, std::shared_ptr<Entity> me, Map *map) {
  if (action.action == eAction::AI) {
    action = StepAI(me, map);
  }
  StepExecutePlan(action, me, map);
}

Output Robot::StepAI(std::shared_ptr<Entity> me, Map *map) {
  Output action;
  attacking_ = false;

  // Target visible ennemi
  auto visible_entities = map->ListVisibleEntities(
      me->position(), Tag::ROBOT_TARGET, Tag::WALL_LIKE, 20);

  std::shared_ptr<Entity> best_target;
  for (auto &e : visible_entities) {
    if (e.get() == this) {
      continue;
    }
    best_target = e;
    break;
  }
  if (best_target) {
    // Move/attack toward target
    attacking_ = true;
    return GoToDirect(Tag::NON_PASSABLE, best_target->position(), map, true);
  }

  // Patrol
  auto maybe_patrol =
      Patrol(Tag::NON_PASSABLE, Tag::WALL_LIKE, EntityType::PATROL_ROUTE, map);
  if (maybe_patrol.has_value()) {
    return maybe_patrol.value();
  }

  // Random walk
  return RandomDirection(Tag::NON_PASSABLE, map);
}

void Robot::StepExecutePlan(Output action, std::shared_ptr<Entity> me,
                            Map *map) {
  switch (action.action) {
    case eAction::MOVE: {
      Vector2i dir(action.move);
      Vector2i new_pos = position() + dir;
      if (!map->Contains(new_pos)) {
        break;
      }
      const auto &cell = map->cell(new_pos);

      bool passable = true;
      for (const auto &e : cell.entities_) {
        if (e->type() == EntityType::CONVEYOR_BELT &&
            dynamic_cast<ConveyorBelt *>(e.get())->direction() ==
                ReverseDirection(action.move)) {
          passable = false;
        }
        if (e->HasTag(Tag::NON_PASSABLE)) {
          passable = false;
        }
      }

      if (!passable) {
        break;
      }
      map->MoveEntity(new_pos, me);
    } break;

    case eAction::MELLE_ATTACK: {
      Vector2i dir(action.move);
      Vector2i new_pos = position() + dir;
      if (!map->Contains(new_pos)) {
        break;
      }
      for (const auto &e : map->cell(new_pos).entities_) {
        if (e->HasTag(Tag::ROBOT_TARGET)) {
          e->Hurt(2, me, e, map);
          break;
        }
      }
    } break;
  }
}

void Goliat::Step(Output action, std::shared_ptr<Entity> me, Map *map) {
  if (action.action == eAction::AI) {
    action = StepAI(me, map);
  }
  StepExecutePlan(action, me, map);
}

Output Goliat::StepAI(std::shared_ptr<Entity> me, Map *map) {
  attacking_ = false;
  Output action;

  // Target visible ennemi
  auto visible_entities = map->ListVisibleEntities(
      me->position(), Tag::ROBOT_TARGET, Tag::WALL_LIKE, 20);

  std::shared_ptr<Entity> best_target;
  for (auto &e : visible_entities) {
    if (e.get() == this) {
      continue;
    }
    best_target = e;
    break;
  }
  if (best_target) {
    // Move/attack toward target
    attacking_ = true;
    return GoToDirect(Tag::NON_PASSABLE, best_target->position(), map, true);
  }

  // Patrol
  auto maybe_patrol =
      Patrol(Tag::NON_PASSABLE, Tag::WALL_LIKE, EntityType::PATROL_ROUTE, map);
  if (maybe_patrol.has_value()) {
    return maybe_patrol.value();
  }

  // Random walk
  return RandomDirection(Tag::NON_PASSABLE, map);
}

void Goliat::StepExecutePlan(Output action, std::shared_ptr<Entity> me,
                             Map *map) {
  switch (action.action) {
    case eAction::MOVE: {
      Vector2i dir(action.move);
      Vector2i new_pos = position() + dir;
      if (!map->Contains(new_pos)) {
        break;
      }
      const auto &cell = map->cell(new_pos);

      bool passable = true;
      for (const auto &e : cell.entities_) {
        if (e->type() == EntityType::CONVEYOR_BELT &&
            dynamic_cast<ConveyorBelt *>(e.get())->direction() ==
                ReverseDirection(action.move)) {
          passable = false;
        }
        if (e->HasTag(Tag::NON_PASSABLE)) {
          passable = false;
        }
      }

      if (!passable) {
        break;
      }
      map->MoveEntity(new_pos, me);
    } break;

    case eAction::MELLE_ATTACK: {
      Vector2i dir(action.move);
      Vector2i new_pos = position() + dir;
      if (!map->Contains(new_pos)) {
        break;
      }
      for (const auto &e : map->cell(new_pos).entities_) {
        if (e->HasTag(Tag::ROBOT_TARGET)) {
          e->Hurt(5, me, e, map);
          break;
        }
      }
    } break;
  }
}

bool Turret::Hurt(int amount, std::shared_ptr<Entity> emiter,
                  std::shared_ptr<Entity> me, Map *map) {
  const auto r = Entity::Hurt(amount, emiter, me, map);
  if (r) {
    CreateExplosion(position(), me, map, 10, 2);
  }
  return r;
}

bool Robot::Hurt(int amount, std::shared_ptr<Entity> emiter,
                 std::shared_ptr<Entity> me, Map *map) {
  const auto r = Entity::Hurt(amount, emiter, me, map);
  if (r) {
    CreateExplosion(position(), me, map, 10, 2);
  }
  return r;
}

bool Goliat::Hurt(int amount, std::shared_ptr<Entity> emiter,
                  std::shared_ptr<Entity> me, Map *map) {
  const auto r = Entity::Hurt(amount, emiter, me, map);
  if (r) {
    CreateExplosion(position(), me, map, 10, 2);
  }
  return r;
}

DisplaySymbol Worm::Display() const {
  terminal::eSymbol symbol = terminal::eSymbol::WORM_BODY;
  if (dir_[0] == 0) {
    symbol = terminal::eSymbol::WORM_HEAD;
  } else if (dir_[1] == 0) {
    symbol = terminal::eSymbol::WORM_TAIL;
  }
  return DisplaySymbol{symbol, 50, .color = terminal::eColor::YELLOW};
}

Output Worm::RandomMove(const Worm::Segments &segments,
                        std::shared_ptr<Worm> head, Map *map) {
  Output action;
  action.action = eAction::MOVE;

  auto gen_rnd_float = [&]() -> float {
    return std::uniform_real_distribution<float>(0, 1)(map->rnd());
  };

  // Don't use this->pos.
  const auto &pos = head->position();

  eDirection last_dir = ReverseDirection((eDirection)head->dir_[1]);
  if (last_dir == 0) {
    return RandomDirection(Tag::NON_PASSABLE_WORM, map);
  }

  auto target_pos = pos + Vector2i(last_dir);
  auto &target_cell = map->cell(target_pos);
  auto target_passable = !target_cell.HasTag(Tag::NON_PASSABLE_WORM);

  const eDirection left_dir = TurnClock(last_dir);
  auto target_pos_left = pos + Vector2i(left_dir);
  auto &target_cell_left = map->cell(target_pos_left);
  auto target_left_passable = !target_cell_left.HasTag(Tag::NON_PASSABLE_WORM);

  const eDirection right_dir = TurnInverseClock(last_dir);
  auto target_pos_right = pos + Vector2i(right_dir);
  auto &target_cell_right = map->cell(target_pos_right);
  auto target_right_passable =
      !target_cell_right.HasTag(Tag::NON_PASSABLE_WORM);

  std::vector<eDirection> possible;
  bool look_side = false;
  if (target_passable) {
    possible.push_back(last_dir);
    if ((map->time() - head->last_non_necessary_turn_) >= 4 &&
        gen_rnd_float() < 0.15) {
      look_side = true;
      head->last_non_necessary_turn_ = map->time();
    }
  } else {
    look_side = true;
  }

  if (look_side) {
    if (target_left_passable) {
      possible.push_back(left_dir);
    }
    if (target_right_passable) {
      possible.push_back(right_dir);
    }
  }

  if (possible.empty()) {
    action.move = eDirection::NONE;
    head->num_blocked++;
    if (head->num_blocked > 4) {
      action.action = eAction::MAGIC;
      action.magic_idx = 0;  // Reverse segments.
    }
    return action;
  }
  head->num_blocked = 0;

  std::shuffle(possible.begin(), possible.end(), map->rnd());
  action.move = possible.front();
  return action;
}

void Worm::ReverseSegments(Segments *segments) {
  std::reverse(segments->begin(), segments->end());
  for (auto &s : *segments) {
    std::swap(s->dir_[0], s->dir_[1]);
  }
}

Output Worm::StepAI(const Worm::Segments &segments, std::shared_ptr<Worm> head,
                    Map *map) {
  // Target visible ennemi
  auto visible_entities = map->ListVisibleEntities(
      head->position(), Tag::WORM_TARGET, Tag::WALL_LIKE, 20);

  std::shared_ptr<Entity> best_target;
  bool best_target_is_low_priority;
  for (auto &e : visible_entities) {
    if (e.get() == this) {
      continue;
    }
    const bool low_priority = e->HasTag(Tag::WORM_LOW_PRIORITY);
    if (!best_target) {
      best_target = e;
      best_target_is_low_priority = low_priority;
    } else if (best_target_is_low_priority && !low_priority) {
      best_target = e;
      best_target_is_low_priority = false;
      break;
    }
  }

  if (best_target) {
    // Move/attack toward target
    last_target_ = best_target->position();
    last_target_time_ = map->time();
    auto output =
        head->GoToDirect(Tag::NON_PASSABLE_WORM, best_target->position(), map,
                         !best_target->HasTag(Tag::DONT_ATTACK));
    return output;
  }

  // Target past visible ennemi
  if (last_target_.has_value()) {
    if (last_target_.value() == position() ||
        (map->time() - last_target_time_) > 20) {
      last_target_ = {};
    } else {
      return head->GoToDirect(Tag::NON_PASSABLE_WORM, last_target_.value(), map,
                              false);
    }
  }
  return RandomMove(segments, head, map);
}

void Worm::MoveSegments(const Vector2i &new_pos, std::shared_ptr<Entity> head,
                        Map *map, Worm::Segments *segments) {
  for (int i = segments->size() - 1; i >= 0; i--) {
    auto &cur = (*segments)[i];

    Vector2i target_pos;
    if (i > 0) {
      target_pos = (*segments)[i - 1]->position();
    } else {
      target_pos = new_pos;
    }

    if (i < segments->size() - 1) {
      auto &next = (*segments)[i + 1];
      eDirection motion_dir = (target_pos - cur->position()).MajorDir();
      cur->dir_[1] = ReverseDirection(motion_dir);
      next->dir_[0] = motion_dir;
    }

    map->MoveEntity(target_pos, cur);
  }

  segments->front()->dir_[0] = eDirection::NONE;
  segments->back()->dir_[1] = eDirection::NONE;
}

void Worm::StepExecutePlan(Output action, std::shared_ptr<Worm> head, Map *map,
                           Worm::Segments *segments) {
  switch (action.action) {
    case eAction::MAGIC: {
      ReverseSegments(segments);
    } break;

    case eAction::MOVE: {
      Vector2i dir(action.move);
      Vector2i new_pos = segments->front()->position() + dir;
      if (!map->Contains(new_pos)) {
        break;
      }
      auto &cell = map->cell(new_pos);

      bool passable = true;
      for (const auto &e : cell.entities_) {
        if (e->type() == EntityType::CONVEYOR_BELT &&
            dynamic_cast<ConveyorBelt *>(e.get())->direction() ==
                ReverseDirection(action.move)) {
          passable = false;
        }
        if (e->HasTag(Tag::NON_PASSABLE_WORM)) {
          passable = false;
        }
      }

      if (!passable) {
        break;
      }

      if (cell.HasTag(Tag::NON_PASSABLE)) {
        for (const auto &e : cell.entities_) {
          if (e->HasTag(Tag::NON_PASSABLE)) {
            map->RemoveEntity(e);
          }
        }
      }

      MoveSegments(new_pos, head, map, segments);
    } break;

    case eAction::MELLE_ATTACK: {
      Vector2i dir(action.move);
      Vector2i new_pos = segments->front()->position() + dir;
      if (!map->Contains(new_pos)) {
        break;
      }
      for (const auto &e : map->cell(new_pos).entities_) {
        if (e->HasTag(Tag::WORM_TARGET)) {
          e->Hurt(3, head, e, map);
          break;
        }
      }
    } break;
  }
}

void Worm::Step(Output action, std::shared_ptr<Entity> me, Map *map) {
  if (last_run_ == map->time()) {
    return;
  }

  bool changed;
  Worm::Segments segments;
  std::tie(segments, changed) = ListSegments(me, map);

  for (auto s : segments) {
    s->last_run_ = map->time();
  }

  if (changed) {
    EqualizeSegmentHp(&segments, map);
  }

  if (segments.size() <= 2) {
    // Too short. Die.
    KillSegments(&segments, map);
    return;
  }

  auto &head = segments.front();

  if (action.action == eAction::AI) {
    action = StepAI(segments, head, map);
  }
  StepExecutePlan(action, head, map, &segments);
}

void Worm::EqualizeSegmentHp(Segments *segments, Map *map) {
  int sum_hp = 0;
  for (const auto &s : *segments) {
    sum_hp += s->hp();
  }

  const int hp_per_segement = sum_hp / segments->size();
  const int last_segment_hp = sum_hp - (segments->size() - 1) * hp_per_segement;
  (*segments)[0]->SetHp(last_segment_hp, (*segments)[0], map);
  for (int i = 1; i < segments->size(); i++) {
    (*segments)[i]->SetHp(hp_per_segement, (*segments)[i], map);
  }
}

void Worm::PrintSegments(const Worm::Segments &segments) {
  LOG(INFO) << "Segments:                 ";
  for (const auto &s : segments) {
    LOG(INFO) << "\tpos:{" << s->position() << "} pred:" << s->dir_[0]
              << " next: " << s->dir_[1] << "                 ";
  }
}

std::pair<Worm::Segments, bool> Worm::ListSegments(std::shared_ptr<Entity> me,
                                                   Map *map) {
  bool changed = false;
  Worm::SegmentList segment_list;
  segment_list.push_back(std::dynamic_pointer_cast<Worm>(me));
  ListSegments(me, map, true, &segment_list, &changed);
  ListSegments(me, map, false, &segment_list, &changed);

  Worm::Segments segments;
  segments.insert(segments.end(), segment_list.begin(), segment_list.end());
  return {segments, changed};
}

void Worm::ListSegments(std::shared_ptr<Entity> me, Map *map, bool next,
                        Worm::SegmentList *segments, bool *changed) {
  const int dir = dir_[next];
  if (dir <= 0) {
    return;
  }
  const auto target_pos = position() + Vector2i(dir);
  const auto &target_cell = map->cell(target_pos);
  for (const auto &e : target_cell.entities_) {
    if (e->type() != EntityType::WORM) {
      continue;
    }
    auto target_worm = std::dynamic_pointer_cast<Worm>(e);
    DCHECK(target_worm);
    if (target_worm->last_run_ == map->time()) {
      continue;
    }
    target_worm->dir_[!next] = ReverseDirection((eDirection)dir);
    if (next) {
      segments->push_back(target_worm);
    } else {
      segments->push_front(target_worm);
    }
    target_worm->ListSegments(target_worm, map, next, segments, changed);
    return;
  }

  *changed = true;
}

void Worm::KillSegments(Segments *segments, Map *map) {
  for (auto &s : *segments) {
    map->RemoveEntity(s);
  }
}

}  // namespace common_game
}  // namespace exploratron
