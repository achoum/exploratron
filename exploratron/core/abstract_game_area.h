#ifndef EXPLORATRON_CORE_ABSTRACT_GAME_ARENA_H_
#define EXPLORATRON_CORE_ABSTRACT_GAME_ARENA_H_

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "exploratron/core/abstract_arena.h"
#include "exploratron/core/abstract_controller.h"
#include "exploratron/core/utils/maths.h"
#include "exploratron/core/utils/register.h"
#include "exploratron/core/utils/terminal.h"

namespace exploratron {
namespace abstract_game_area {
using eColor = terminal::eColor;

class Map;
class AbstractGameArena;

struct DisplaySymbol {
  int symbol_; // The first 256 values are characters. The next ones are
               // symbols.
  int priotity_;
  bool visible_ = true;
  bool help_ = true;
  eColor color = terminal::eColor::WHITE;
};

struct Action {
  int idx;
  std::string label;
  char shortcut;
  // If >0, enable targeting with radius.
  int target_radius;
};

class Entity {
public:
  Entity() : hp_(1) {}
  Entity(int hp) : hp_(hp) { DCHECK_GT(hp_, 0); }
  virtual ~Entity() = default;

  virtual int type() const = 0;
  virtual DisplaySymbol Display() const = 0;
  virtual std::vector<int> Tags() const = 0;
  virtual void Step(Output action, std::shared_ptr<Entity> me, Map *map) {}
  virtual std::string Name() const = 0;

  // Return true is the entity is destroyed.
  virtual bool Hurt(int amount, std::shared_ptr<Entity> emiter,
                    std::shared_ptr<Entity> me, Map *map);

  const Vector2i &position() const { return position_; }
  bool contolled() const { return contolled_; }
  int hp() const { return hp_; }
  void SetHp(int value, std::shared_ptr<Entity> me, Map *map);
  void SetControlled(bool value) { contolled_ = value; }
  int id() const { return id_; }
  int born_step() const { return born_step_; }
  bool HasTag(int tag) const;

  Output RandomDirection(int blocking_tag, Map *map);
  Output GoToDirect(int blocking_tag, Vector2i dst, Map *map,
                    bool melle_attack);
  std::optional<Output> Patrol(int blocking_tag, int not_visible_tag,
                               int patrol_type, Map *map);
  virtual std::string status() const { return "hp:" + std::to_string(hp_); }
  virtual std::vector<Action> AvailableMagics() const { return {}; }
  bool removed() const { return removed_; }

  virtual void ReceiveSignal(int signal, std::shared_ptr<Entity> me, Map *map) {
  }

  int last_conveyor_move_time_ = -1;

private:
  int id_ = -1;
  int born_step_ = -1;
  bool contolled_ = false;
  Vector2i position_;
  bool removed_ = false;
  int hp_;
  eDirection last_patrol_dir_ = eDirection::NONE;

  friend class Map;
};

class Cell {
public:
  bool HasTag(int tag);
  std::shared_ptr<Entity> HasEntity(int type);

  std::vector<std::shared_ptr<Entity>> entities_;
};

class Map {
public:
  Map(AbstractGameArena *parent, Vector2i size);

  Cell &cell(Vector2i p) { return cells_[CellIdx(p)]; }

  const Cell &cell(Vector2i p) const { return cells_[CellIdx(p)]; }

  int CellIdx(Vector2i p) const {
    DCHECK(size_.PointIsInRect(p)) << " p:" << p;
    return p.x + p.y * size_.x;
  }

  int NumCells() const { return cells_.size(); }

  void Draw() const;
  void Step(const Output &control);

  bool Contains(Vector2i p) const { return size_.PointIsInRect(p); }

  void AddEntity(const Vector2i &pos, std::shared_ptr<Entity> entity);
  void RemoveEntity(std::shared_ptr<Entity> entity);
  void MoveEntity(Vector2i new_pos, std::shared_ptr<Entity> entity);

  std::vector<std::shared_ptr<Entity>> ListEntitiesWithTag(int filter_tag);

  std::vector<std::shared_ptr<Entity>> ListVisibleEntities(Vector2i pos,
                                                           int filter_tag,
                                                           int not_visible_tag,
                                                           int max_dist);

  std::vector<std::shared_ptr<Entity>>
  ListVisibleEntitiesByType(Vector2i pos, int entity_type, int not_visible_tag,
                            int max_dist);

  bool LineWithoutTag(Vector2i p1, Vector2i p2, int tag);
  void IterateLine(Vector2i p1, Vector2i p2,
                   const std::function<bool(const Vector2i &p)> &callback);

  std::optional<Vector2i> RandomNonOccupiedCell(std::mt19937_64 *rnd) const;

  int time() const { return time_; }
  std::mt19937_64 &rnd() { return rnd_; }
  void AddLog(std::string log);
  std::vector<std::shared_ptr<Entity>> ControlledEntities();
  void ApplyPending();
  void Explode(Vector2i pos, int radius,
               std::function<bool(const Vector2i &)> explore);

private:
  void AddEntityImplem(std::shared_ptr<Entity> entity);
  void RemoveEntityImplem(std::shared_ptr<Entity> entity);
  void MoveEntityImplem(Vector2i new_pos, std::shared_ptr<Entity> entity);

  std::vector<std::shared_ptr<Entity>> pending_to_add_;
  std::vector<std::shared_ptr<Entity>> pending_to_remove_;
  std::vector<std::pair<Vector2i, std::shared_ptr<Entity>>> pending_to_move_;

  Vector2i size_;
  std::vector<Cell> cells_;
  std::vector<std::shared_ptr<Entity>> entities_;
  int next_entity_id_ = 0;
  int time_ = 0;
  std::mt19937_64 rnd_;
  AbstractGameArena *parent_;

  std::shared_ptr<Entity> last_controlled_;

  friend class AbstractGameArena;
};

class AbstractGameArena : public AbstractArena {
public:
  AbstractGameArena(
      const std::vector<const AbstractControllerBuilder *> &controller_builders,
      const MapDef &map_definition);
  virtual ~AbstractGameArena() = default;
  virtual std::string Info() const override { return "AbstractGameArena"; };

  bool Step() override;
  void Draw() const override;
  void Initialize(Vector2i size);
  void AddEntity(const Vector2i &pos, std::shared_ptr<Entity> entity);
  void AddLog(std::string log) { logs_.push_back(log); }
  Map &map() { return *map_; }

  Scores FinalScore() const override { return {0}; }
  std::unique_ptr<Map> map_;
  std::unique_ptr<AbstractController> controller_;
  std::vector<std::string> logs_;

private:
};

struct EntityDef {
  std::function<std::shared_ptr<Entity>()> builder;
};

inline std::unordered_map<int, EntityDef> global_registered_entities;

#define REGISTER_ENTITY(X)                                                     \
  inline struct Register##X {                                                  \
    Register##X() {                                                            \
      ::exploratron::abstract_game_area::EntityDef def;              \
      def.builder = []() -> std::shared_ptr<Entity> {                          \
        return std::make_shared<X>();                                          \
      };                                                                       \
      ::exploratron::abstract_game_area::global_registered_entities  \
          [def.builder()->type()] = def;                                       \
    }                                                                          \
  } register_##X;

} // namespace abstract_game_area
} // namespace exploratron
#endif