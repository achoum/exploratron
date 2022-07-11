#ifndef EXPLORATRON_AREA_COMMON_GAME_H_
#define EXPLORATRON_AREA_COMMON_GAME_H_

#include <functional>
#include <memory>
#include <optional>
#include <random>
#include <vector>

#include "absl/strings/str_cat.h"
#include "exploratron/core/abstract_arena.h"
#include "exploratron/core/abstract_controller.h"
#include "exploratron/core/abstract_game_area.h"
#include "exploratron/core/utils/terminal.h"

namespace exploratron {
namespace common_game {

using Entity = abstract_game_area::Entity;
using DisplaySymbol = abstract_game_area::DisplaySymbol;
using AbstractGameArena = abstract_game_area::AbstractGameArena;
using Map = abstract_game_area::Map;
using Action = abstract_game_area::Action;

enum SignalType : int {
  PRESS = 0,
  RECEIVE_ELETRICITY = 1,
};

enum EntityType : int {
  WALL = 0,
  SOFT_WALL = 1,
  FUNGUS = 2,
  FUNGUS_TOWER = 3,
  ANT = 4,
  PATROL_ROUTE = 5,
  PLAYER = 6,
  WATER = 7,
  FIRE = 8,
  FOOD = 9,
  EXIT_DOOR = 10,
  PHEROMONE = 11,
  EXPLOSIVE = 12,
  EXPLOSION = 13,
  UNBREAKABLE_WALL = 14,
  BOULDER = 15,
  ANT_QUEEN = 16,
  STEEL_DOOR = 17,
  BUTTON = 18,
  CONVEYOR_BELT = 19,
  STEEL_WALL = 20,
  EXPLOSIVE_BAREL = 21,
  WIRE = 22,
  PROXY = 23,
  MESSAGE = 24,
  TURRET = 25,
  ROBOT = 26,
  GOLIAT = 27,
  LASER = 28,
  WORM = 29,
};

enum Tag {
  // Block motions.
  NON_PASSABLE,
  // A floor (not used yet);
  FLOOR_LIKE,
  // Block fireballs, fungus, explosion propagation.
  WALL_LIKE,
  // A mob (not used yet).
  MOB,
  // Hurt by ant.
  ANT_TARGET,
  // Hurt target by hand is not target without low priority are found.
  ANT_LOW_PRIORITY,
  // Hurt by player.
  PLAYER_TARGET,
  // Hurt by fungus.
  FUNGUS_TARGET,
  // Hurt by fire.
  FIRE_TARGET,
  // Fire on a flamable tile will spread to neighbor flamable tiles.
  FLAMABLE,
  // Targets might be attracted but will not attack.
  DONT_ATTACK,
  // Familly of fungus
  FUNGUS_LIKE,
  // Can be destroyed by explosion.
  EXPLOSION_TARGET,
  // Can be pushed.
  PUSHABLE,
  // Can be pickedup
  ITEM,
  // When hurting food, consuder that the food is eaten.
  ATTACK_IS_EAT_FOOD,
  // Move by a conveyor belt.
  MOVED_BY_CONVEYOR_BELT,
  // Can be pressed by the player.
  ACTIONABLE,
  // Can receive an eletric signal.
  RECEIVE_ELETRIC_SIGNAL,
  // Will trigger a proxy sensor.
  TRIGGER_PROXY_SENSOR,
  // Will active a button if pushed by a conveyor belt.
  CAN_ACTION_BUTTON_ON_CONVEYOR_BELT,
  // Will be hurted by a door closed on it.
  KILLED_BY_AUTOMATIC_METAL_DOOR,
  // Targeted by robots
  ROBOT_TARGET,
};

class UnbreakableWall : public Entity {
public:
  int type() const override { return EntityType::UNBREAKABLE_WALL; }
  std::string Name() const override { return "unbreakable wall"; }
  DisplaySymbol Display() const override {
    return DisplaySymbol{256 + 0, 40, .help_ = false,
                         .color = terminal::eColor::GRAY};
  }
  std::vector<int> Tags() const override {
    return {Tag::WALL_LIKE, Tag::NON_PASSABLE};
  }
};
REGISTER_ENTITY(UnbreakableWall);

class SteelWall : public Entity {
public:
  int type() const override { return EntityType::STEEL_WALL; }
  std::string Name() const override { return "steel wall"; }
  DisplaySymbol Display() const override {
    return DisplaySymbol{256 + 0, 40, .color = terminal::eColor::BLUE};
  }
  std::vector<int> Tags() const override {
    return {Tag::WALL_LIKE, Tag::NON_PASSABLE};
  }
};
REGISTER_ENTITY(SteelWall);

class Wall : public Entity {
public:
  int type() const override { return EntityType::WALL; }
  std::string Name() const override { return "wall"; }
  DisplaySymbol Display() const override {
    return DisplaySymbol{256 + 0, 40, .color = terminal::eColor::GRAY};
  }
  std::vector<int> Tags() const override {
    return {Tag::WALL_LIKE, Tag::NON_PASSABLE, Tag::EXPLOSION_TARGET};
  }
};
REGISTER_ENTITY(Wall);

class SoftWall : public Entity {
public:
  SoftWall() : Entity(2) {}
  int type() const override { return EntityType::SOFT_WALL; }
  std::string Name() const override { return "soft wall"; }
  DisplaySymbol Display() const override { return DisplaySymbol{256 + 0, 0}; }
  std::vector<int> Tags() const override {
    return {Tag::WALL_LIKE, Tag::NON_PASSABLE, Tag::PLAYER_TARGET,
            Tag::EXPLOSION_TARGET};
  }
};
REGISTER_ENTITY(SoftWall);

class AutomaticDoor : public Entity {
public:
  int type() const override { return EntityType::STEEL_DOOR; }
  std::string Name() const override { return "automatic door"; }
  DisplaySymbol Display() const override {
    return DisplaySymbol{closed_ ? '+' : '-', 3,
                         .color = terminal::eColor::BLUE};
  }
  std::vector<int> Tags() const override {
    if (closed_) {
      return {Tag::WALL_LIKE, Tag::NON_PASSABLE, Tag::RECEIVE_ELETRIC_SIGNAL};
    } else {
      return {Tag::RECEIVE_ELETRIC_SIGNAL};
    }
  }
  void Step(Output action, std::shared_ptr<Entity> me, Map *map) override;
  void ReceiveSignal(int signal, std::shared_ptr<Entity> me, Map *map) override;

private:
  bool closed_ = true;
};
REGISTER_ENTITY(AutomaticDoor);

class Fungus : public Entity {
public:
  Fungus() {}
  Fungus(int left) : left_(left) {}
  int type() const override { return EntityType::FUNGUS; }
  std::string Name() const override { return "fungus"; }
  DisplaySymbol Display() const override {
    return DisplaySymbol{'"', 0, .color = terminal::eColor::GRAY};
  }
  std::vector<int> Tags() const override {
    return {Tag::FUNGUS_LIKE, Tag::FIRE_TARGET, Tag::FLAMABLE,
            Tag::EXPLOSION_TARGET, Tag::MOVED_BY_CONVEYOR_BELT};
  }
  void Step(Output action, std::shared_ptr<Entity> me, Map *map) override;

private:
  int left_ = 10;
};
REGISTER_ENTITY(Fungus);

class FungusTower : public Entity {
public:
  std::string Name() const override { return "fungus tower"; }
  int type() const override { return EntityType::FUNGUS_TOWER; }
  DisplaySymbol Display() const override { return DisplaySymbol{'f', 0}; }
  std::vector<int> Tags() const override {
    return {
        Tag::PLAYER_TARGET,    Tag::FUNGUS_LIKE,
        Tag::FIRE_TARGET,      Tag::FLAMABLE,
        Tag::EXPLOSION_TARGET, Tag::MOVED_BY_CONVEYOR_BELT}; // Tag::ANT_TARGET,
  }
  void Step(Output action, std::shared_ptr<Entity> me, Map *map) override;
};
// REGISTER_ENTITY(FungusTower);

class Ant : public Entity {
public:
  Ant() : Entity(2) {}
  int type() const override { return EntityType::ANT; }
  std::string Name() const override { return "ant"; }
  DisplaySymbol Display() const override {
    return DisplaySymbol{'a', 50, .color = terminal::eColor::RED};
  }
  std::vector<int> Tags() const override {
    return {
        Tag::MOB,
        Tag::NON_PASSABLE,
        Tag::PLAYER_TARGET,
        Tag::FUNGUS_TARGET,
        Tag::FIRE_TARGET,
        Tag::EXPLOSION_TARGET,
        Tag::ATTACK_IS_EAT_FOOD,
        Tag::MOVED_BY_CONVEYOR_BELT,
        Tag::CAN_ACTION_BUTTON_ON_CONVEYOR_BELT,
        Tag::TRIGGER_PROXY_SENSOR,
        Tag::KILLED_BY_AUTOMATIC_METAL_DOOR,
        Tag::ROBOT_TARGET,
    };
  }
  void Step(Output action, std::shared_ptr<Entity> me, Map *map) override;

  void SetTarget(Vector2i pos, Map *map) {
    last_target_ = pos;
    last_target_time_ = map->time();
  }

private:
  Output StepAI(std::shared_ptr<Entity> me, Map *map);
  void StepExecutePlan(Output action, std::shared_ptr<Entity> me, Map *map);

  std::optional<Vector2i> last_target_;
  int last_target_time_ = 0;
  int last_pheromone_time_ = 0;
  eDirection last_pheromone_dir_ = eDirection::NONE;
};
REGISTER_ENTITY(Ant);

class AntQueen : public Entity {
public:
  AntQueen() : Entity(10) {}
  int type() const override { return EntityType::ANT_QUEEN; }
  std::string Name() const override { return "ant queen"; }
  DisplaySymbol Display() const override {
    return DisplaySymbol{'A', 50, .color = terminal::eColor::RED};
  }
  std::vector<int> Tags() const override {
    return {
        Tag::MOB,
        Tag::NON_PASSABLE,
        Tag::PLAYER_TARGET,
        Tag::FUNGUS_TARGET,
        Tag::FIRE_TARGET,
        Tag::EXPLOSION_TARGET,
        Tag::MOVED_BY_CONVEYOR_BELT,
        Tag::CAN_ACTION_BUTTON_ON_CONVEYOR_BELT,
        Tag::TRIGGER_PROXY_SENSOR,
        Tag::KILLED_BY_AUTOMATIC_METAL_DOOR,
        Tag::ROBOT_TARGET,
    };
  }
  void Step(Output action, std::shared_ptr<Entity> me, Map *map) override;
  std::string status() const override {
    return Entity::status() + " s:" + std::to_string(time_to_spwan_);
  }
  std::vector<Action> AvailableMagics() const override;

private:
  enum eMagic {
    CREATE_ANT,
  };

  Output StepAI(std::shared_ptr<Entity> me, Map *map);
  void StepExecutePlan(Output action, std::shared_ptr<Entity> me, Map *map);

  int time_to_spwan_ = 0;
};
REGISTER_ENTITY(AntQueen);

class PatrolRoute : public Entity {
public:
  PatrolRoute() {}
  int type() const override { return EntityType::PATROL_ROUTE; }
  std::string Name() const override { return "patrol route"; }
  DisplaySymbol Display() const override {
    return DisplaySymbol{' ', -1000, .help_ = false};
  }
  std::vector<int> Tags() const override { return {}; }
};
REGISTER_ENTITY(PatrolRoute);

class Player : public Entity {
public:
  Player() : Entity(5) {}
  int type() const override { return EntityType::PLAYER; }
  std::string Name() const override { return "player"; }
  DisplaySymbol Display() const override {
    return DisplaySymbol{256 + 2, 100, .color = terminal::eColor::VIOLET};
  } // @
  std::vector<int> Tags() const override {
    return {
        Tag::NON_PASSABLE,         Tag::ANT_TARGET,
        Tag::FUNGUS_TARGET,        Tag::FIRE_TARGET,
        Tag::EXPLOSION_TARGET,     Tag::MOVED_BY_CONVEYOR_BELT,
        Tag::TRIGGER_PROXY_SENSOR, Tag::KILLED_BY_AUTOMATIC_METAL_DOOR,
        Tag::ROBOT_TARGET,
    };
  }
  void Step(Output action, std::shared_ptr<Entity> me, Map *map) override;
  void StepMove(Output action, std::shared_ptr<Entity> me, Map *map);
  void StepMagic(Output action, std::shared_ptr<Entity> me, Map *map);
  int energy() const { return energy_; }

  std::string status() const override {
    return absl::StrCat(Entity::status(), " mp:", energy(),
                        "  food:", num_food_, " explo:", num_explosive_);
  }
  std::vector<Action> AvailableMagics() const override;

private:
  std::optional<Vector2i> ThrowPosition(Output action,
                                        std::shared_ptr<Entity> me, Map *map);

  enum eMagic {
    FIREBALL,
    THROW_FOOD,
    EXPLOSIVE,
  };

  int energy_ = 5;
  bool has_fireball_ = true;
  int num_food_ = 0;
  int num_explosive_ = 0;
};
REGISTER_ENTITY(Player);

class Water : public Entity {
public:
  Water() {}
  int type() const override { return EntityType::WATER; }
  std::string Name() const override { return "water"; }
  DisplaySymbol Display() const override {
    return DisplaySymbol{'~', 10, .color = terminal::eColor::BLUE};
  }
  std::vector<int> Tags() const override { return {Tag::EXPLOSION_TARGET}; }
  void Step(Output action, std::shared_ptr<Entity> me, Map *map) override;

private:
  // int amount_ = 10;
};
// REGISTER_ENTITY(Water);

class Fire : public Entity {
public:
  int type() const override { return EntityType::FIRE; }
  std::string Name() const override { return "fire"; }
  DisplaySymbol Display() const override {
    return DisplaySymbol{'^', 40, .color = terminal::eColor::YELLOW};
  }
  std::vector<int> Tags() const override {
    return {
        Tag::EXPLOSION_TARGET,
        Tag::KILLED_BY_AUTOMATIC_METAL_DOOR,
    };
  }
  void Step(Output action, std::shared_ptr<Entity> me, Map *map) override;
};
REGISTER_ENTITY(Fire);

class Food : public Entity {
public:
  int type() const override { return EntityType::FOOD; }
  std::string Name() const override { return "food"; }
  DisplaySymbol Display() const override {
    return DisplaySymbol{'f', 20, .color = terminal::eColor::GREEN};
  }
  std::vector<int> Tags() const override {
    return {
        Tag::FIRE_TARGET,
        Tag::ANT_TARGET,
        Tag::EXPLOSION_TARGET,
        Tag::ITEM,
        Tag::ANT_LOW_PRIORITY,
        Tag::MOVED_BY_CONVEYOR_BELT,
        Tag::KILLED_BY_AUTOMATIC_METAL_DOOR,
    };
  }
  void Step(Output action, std::shared_ptr<Entity> me, Map *map) override;
  bool Hurt(int amount, std::shared_ptr<Entity> emiter,
            std::shared_ptr<Entity> me, Map *map) override;

private:
  int last_time_eaten_ = -1;
  int amount_left = 100;
};
REGISTER_ENTITY(Food);

class ExitDoor : public Entity {
public:
  int type() const override { return EntityType::EXIT_DOOR; }
  std::string Name() const override { return "exit stairs"; }
  DisplaySymbol Display() const override {
    return DisplaySymbol{'>', 10, .color = terminal::eColor::GREEN};
  }
  std::vector<int> Tags() const override {
    return {Tag::MOVED_BY_CONVEYOR_BELT};
  }
};
REGISTER_ENTITY(ExitDoor);

class Pheromone : public Entity {
public:
  Pheromone() {}
  Pheromone(eDirection dir) : dir_(dir) {}
  int type() const override { return EntityType::PHEROMONE; }
  std::string Name() const override { return "pheromone"; }
  DisplaySymbol Display() const override { return DisplaySymbol{'\'', 0}; }
  std::vector<int> Tags() const override {
    return {Tag::FIRE_TARGET, Tag::EXPLOSION_TARGET};
  }
  void Step(Output action, std::shared_ptr<Entity> me, Map *map) override;
  eDirection dir() const { return dir_; }

private:
  eDirection dir_ = eDirection::NONE;
};
REGISTER_ENTITY(Pheromone);

class Explosive : public Entity {
public:
  Explosive() {}
  Explosive(bool active) : active_(active) {}

  int type() const override { return EntityType::EXPLOSIVE; }
  std::string Name() const override { return "dynamite"; }
  DisplaySymbol Display() const override;
  std::vector<int> Tags() const override;
  void Step(Output action, std::shared_ptr<Entity> me, Map *map) override;
  bool Hurt(int amount, std::shared_ptr<Entity> emiter,
            std::shared_ptr<Entity> me, Map *map) override;

private:
  void Explode(std::shared_ptr<Entity> me, Map *map);

  int left_ = 5;
  bool active_ = false;
};
REGISTER_ENTITY(Explosive);

class Explosion : public Entity {
public:
  int type() const override { return EntityType::EXPLOSION; }
  std::string Name() const override { return "explosion"; }
  DisplaySymbol Display() const override {
    return DisplaySymbol{'*', 10, .help_ = false,
                         .color = terminal::eColor::YELLOW};
  }
  std::vector<int> Tags() const override { return {}; }
  void Step(Output action, std::shared_ptr<Entity> me, Map *map) override;
};
REGISTER_ENTITY(Explosion);

class Boulder : public Entity {
public:
  int type() const override { return EntityType::BOULDER; }
  std::string Name() const override { return "boulder"; }
  DisplaySymbol Display() const override { return DisplaySymbol{'O', 50}; }
  std::vector<int> Tags() const override {
    return {
        Tag::WALL_LIKE,
        Tag::NON_PASSABLE,
        Tag::EXPLOSION_TARGET,
        Tag::PUSHABLE,
        Tag::MOVED_BY_CONVEYOR_BELT,
        Tag::CAN_ACTION_BUTTON_ON_CONVEYOR_BELT,
        Tag::TRIGGER_PROXY_SENSOR,
    };
  }
};
REGISTER_ENTITY(Boulder);

class Button : public Entity {
public:
  int type() const override { return EntityType::BUTTON; }
  std::string Name() const override { return "button"; }
  DisplaySymbol Display() const override { return DisplaySymbol{256 + 5, 50}; }
  std::vector<int> Tags() const override {
    return {Tag::MOVED_BY_CONVEYOR_BELT, Tag::ACTIONABLE, Tag::NON_PASSABLE};
  }
  void ReceiveSignal(int signal, std::shared_ptr<Entity> me, Map *map) override;
};
REGISTER_ENTITY(Button);

class ConveyorBelt : public Entity {
public:
  ConveyorBelt() {}
  ConveyorBelt(int direction) : direction_(direction) {}
  int type() const override { return EntityType::CONVEYOR_BELT; }
  std::string Name() const override { return "conveyor belt"; }
  DisplaySymbol Display() const override;
  std::vector<int> Tags() const override { return {}; }
  void Step(Output action, std::shared_ptr<Entity> me, Map *map) override;
  int direction() const { return direction_; }

private:
  int direction_ = eDirection::RIGHT;
};
REGISTER_ENTITY(ConveyorBelt);

class ExplosiveBarel : public Entity {
public:
  int type() const override { return EntityType::EXPLOSIVE_BAREL; }
  std::string Name() const override { return "explosive barel"; }
  void Step(Output action, std::shared_ptr<Entity> me, Map *map) override;
  DisplaySymbol Display() const override;
  std::vector<int> Tags() const override {
    return {
        Tag::WALL_LIKE,
        Tag::NON_PASSABLE,
        Tag::FIRE_TARGET,
        Tag::EXPLOSION_TARGET,
        Tag::PUSHABLE,
        Tag::MOVED_BY_CONVEYOR_BELT,
        Tag::CAN_ACTION_BUTTON_ON_CONVEYOR_BELT,
        Tag::TRIGGER_PROXY_SENSOR,
        Tag::KILLED_BY_AUTOMATIC_METAL_DOOR,
    };
  }
  bool Hurt(int amount, std::shared_ptr<Entity> emiter,
            std::shared_ptr<Entity> me, Map *map) override;

private:
  void Explode(std::shared_ptr<Entity> me, Map *map);
  int left_ = 5;
  bool active_ = false;
};
REGISTER_ENTITY(ExplosiveBarel);

class Wire : public Entity {
public:
  int type() const override { return EntityType::WIRE; }
  std::string Name() const override { return "wire"; }
  DisplaySymbol Display() const override {
    return DisplaySymbol{'.', 0, .visible_ = false, .help_ = false,
                         .color = terminal::eColor::GRAY};
  }
  std::vector<int> Tags() const override { return {}; }
};
REGISTER_ENTITY(Wire);

class ProxySensor : public Entity {
public:
  int type() const override { return EntityType::PROXY; }
  std::string Name() const override { return "proxy sensor"; }
  DisplaySymbol Display() const override { return DisplaySymbol{'p', 50}; }
  std::vector<int> Tags() const override {
    return {Tag::MOVED_BY_CONVEYOR_BELT, Tag::ACTIONABLE, Tag::NON_PASSABLE};
  }
  void Step(Output action, std::shared_ptr<Entity> me, Map *map) override;

private:
  std::vector<int> last_entity_ids_;
};
REGISTER_ENTITY(ProxySensor);

class Message : public Entity {
public:
  Message() {}
  Message(const std::string &message) : message_(message) {}
  int type() const override { return EntityType::MESSAGE; }
  std::string Name() const override { return "message"; }
  DisplaySymbol Display() const override { return DisplaySymbol{'?', 0}; }
  std::vector<int> Tags() const override {
    return {Tag::MOVED_BY_CONVEYOR_BELT};
  }
  const std::string &message() const { return message_; }
  void Step(Output action, std::shared_ptr<Entity> me, Map *map) override;

private:
  std::vector<int> last_entity_ids_;
  std::string message_;
};
REGISTER_ENTITY(Message);

class Turret : public Entity {
public:
  Turret() : Entity(10) {}
  int type() const override { return EntityType::TURRET; }
  std::string Name() const override { return "turret"; }
  DisplaySymbol Display() const override {
    return DisplaySymbol{'t', 40,
                         .color = (attack_left_ > 0) ? terminal::eColor::RED
                                                     : terminal::eColor::BLUE};
  }
  std::vector<int> Tags() const override {
    return {
        Tag::MOB,
        Tag::NON_PASSABLE,
        Tag::EXPLOSION_TARGET,
        Tag::CAN_ACTION_BUTTON_ON_CONVEYOR_BELT,
        Tag::MOVED_BY_CONVEYOR_BELT,
        Tag::TRIGGER_PROXY_SENSOR,
        Tag::KILLED_BY_AUTOMATIC_METAL_DOOR,
        Tag::ANT_TARGET,
    };
  }
  void Step(Output action, std::shared_ptr<Entity> me, Map *map) override;

  bool Hurt(int amount, std::shared_ptr<Entity> emiter,
            std::shared_ptr<Entity> me, Map *map) override;

private:
  int attack_left_ = 0;
  int attack_dir = 1;
};
REGISTER_ENTITY(Turret);

class Robot : public Entity {
public:
  Robot() : Entity(10) {}
  int type() const override { return EntityType::ROBOT; }
  std::string Name() const override { return "robot"; }
  DisplaySymbol Display() const override {
    return DisplaySymbol{'r', 40,
                         .color = (attacking_ > 0) ? terminal::eColor::RED
                                                   : terminal::eColor::BLUE};
  }
  std::vector<int> Tags() const override {
    return {
        Tag::MOB,
        Tag::NON_PASSABLE,
        Tag::EXPLOSION_TARGET,
        Tag::CAN_ACTION_BUTTON_ON_CONVEYOR_BELT,
        Tag::MOVED_BY_CONVEYOR_BELT,
        Tag::TRIGGER_PROXY_SENSOR,
        Tag::KILLED_BY_AUTOMATIC_METAL_DOOR,
        Tag::ANT_TARGET,
    };
  }
  void Step(Output action, std::shared_ptr<Entity> me, Map *map) override;

  bool Hurt(int amount, std::shared_ptr<Entity> emiter,
            std::shared_ptr<Entity> me, Map *map) override;

private:
  Output StepAI(std::shared_ptr<Entity> me, Map *map);
  void StepExecutePlan(Output action, std::shared_ptr<Entity> me, Map *map);
  bool attacking_ = false;
};
REGISTER_ENTITY(Robot);

class Goliat : public Entity {
public:
  Goliat() : Entity(30) {}
  int type() const override { return EntityType::GOLIAT; }
  std::string Name() const override { return "goliat"; }
  DisplaySymbol Display() const override {
    return DisplaySymbol{'G', 40,
                         .color = (attacking_ > 0) ? terminal::eColor::RED
                                                   : terminal::eColor::BLUE};
  }
  std::vector<int> Tags() const override {
    return {
        Tag::MOB,
        Tag::NON_PASSABLE,
        Tag::EXPLOSION_TARGET,
        Tag::CAN_ACTION_BUTTON_ON_CONVEYOR_BELT,
        Tag::MOVED_BY_CONVEYOR_BELT,
        Tag::TRIGGER_PROXY_SENSOR,
        Tag::ANT_TARGET,
    };
  }
  void Step(Output action, std::shared_ptr<Entity> me, Map *map) override;

  bool Hurt(int amount, std::shared_ptr<Entity> emiter,
            std::shared_ptr<Entity> me, Map *map) override;

private:
  Output StepAI(std::shared_ptr<Entity> me, Map *map);
  void StepExecutePlan(Output action, std::shared_ptr<Entity> me, Map *map);
  bool attacking_ = false;
};
REGISTER_ENTITY(Goliat);

class Laser : public Entity {
public:
  Laser() {}
  Laser(int dir) : dir_(dir) {}
  int type() const override { return EntityType::LASER; }
  std::string Name() const override { return "laser"; }
  DisplaySymbol Display() const override;
  std::vector<int> Tags() const override { return {}; }
  void Step(Output action, std::shared_ptr<Entity> me, Map *map) override;
  bool TestPos(const Vector2i &pos, std::shared_ptr<Entity> me, Map *map);

private:
  int dir_ = 1;
};
REGISTER_ENTITY(Laser);

class Worm : public Entity {
public:
  Worm() : Entity(20) {}
  int type() const override { return EntityType::WORM; }
  std::string Name() const override { return "worm"; }
  DisplaySymbol Display() const override;
  std::vector<int> Tags() const override {
    return {
        Tag::MOB,
        Tag::NON_PASSABLE,
        Tag::PLAYER_TARGET,
        Tag::FUNGUS_TARGET,
        Tag::FIRE_TARGET,
        Tag::EXPLOSION_TARGET,
        Tag::TRIGGER_PROXY_SENSOR,
        Tag::KILLED_BY_AUTOMATIC_METAL_DOOR,
        Tag::ROBOT_TARGET,
    };
  }
  void Step(Output action, std::shared_ptr<Entity> me, Map *map) override;

private:
  typedef std::vector<std::shared_ptr<Entity>> Segments;

  void MoveSegments(const Vector2i &new_pos, std::shared_ptr<Entity> me,
                    Map *map, Segments *segments);
  Segments ListSegments();
   Segments AutoListSegments();
  void KillSegments(Segments *segments, Map *map);
  Output StepAI(const Segments &segments, std::shared_ptr<Entity> me, Map *map);
  void StepExecutePlan(Output action, std::shared_ptr<Entity> me, Map *map,
                       Segments *segments);

  int last_run_ = -1;
  int prev_dir_ = -1;
  int next_dir_ = -1;
};
REGISTER_ENTITY(Worm);

void InitializeFromPng(std::string_view path,
                       std::function<void(Vector2i pos, RGB color)> builder,
                       AbstractGameArena *arena);
void InitializeFromPng(std::string_view path, AbstractGameArena *arena);

void InitializeFromTmx(
    std::string_view path,
    std::function<void(Vector2i pos, int symbol, const std::string &message)>
        builder,
    AbstractGameArena *arena);
void InitializeFromTmx(std::string_view path, AbstractGameArena *arena);

void CreateExplosion(const Vector2i &position, std::shared_ptr<Entity> me,
                     Map *map, int damages, int radius);

void SendSignal(Vector2i pos, Map *map, int signal);

} // namespace common_game
} // namespace exploratron
#endif