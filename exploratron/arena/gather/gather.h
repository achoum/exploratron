#ifndef EXPLORATRON_AREA_GATHER_H_
#define EXPLORATRON_AREA_GATHER_H_

#include <memory>
#include <random>

#include "exploratron/core/abstract_arena.h"
#include "exploratron/core/abstract_controller.h"

namespace exploratron {
namespace gather_area {

enum class CellContent {
  NONE,
  WALL,
  FOOD,
  CHEST,
  COIN,
  CONTROLLER,
  _NUM_TYPES,
};

struct Cell {
  CellContent content = CellContent::NONE;
};

struct Options {
  // Options
  int step_left_ = 300;
  int width_ = 20;  // 20
  int height_ = 10; // 20

  int max_food = 0; // 5
  int max_coins = 1;
};

class GatherArena : public AbstractArena {
public:
  GatherArena(
      const std::vector<const AbstractControllerBuilder *> &controller_builders,
      const Options &options);

  virtual ~GatherArena() = default;
  bool Step() override;
  Scores FinalScore() const override;
  void Draw() const override;

private:
  Cell &cell(Vector2i p) { return cells_[p.x + p.y * options_.width_]; }
  const Cell &cell(Vector2i p) const {
    return cells_[p.x + p.y * options_.width_];
  }

  void SetPlayerPos(Vector2i p, bool check_previous = true);
  void FillCells(Vector2i corner, Vector2i size, CellContent type);
  bool AddRandomCell(CellContent type, int border = 1);
  void FillInput();

  Options options_;

  std::unique_ptr<AbstractController> controller_;
  std::vector<Cell> cells_;
  Vector2i player_;
  int num_food = 0;
  int num_coins = 0;
  int score = 0;
  std::mt19937_64 rnd;
  MapDef map_def;
  Input input;
  int last_score_step = -1;
};

class GatherArenaBuilder : public AbstractArenaBuilder {
public:
  GatherArenaBuilder() {}
  virtual ~GatherArenaBuilder() = default;

  std::unique_ptr<AbstractArena> Create(
      const std::vector<const AbstractControllerBuilder *> &controller_builders)
      const override {
    return std::make_unique<GatherArena>(controller_builders, options_);
  }

  Options options_;

  MapDef MapDefinition() const override;

  std::string name() const override { return "GatherArenaBuilder"; }
};

} // namespace gather_area

REGISTER_AbstractArenaBuilder(gather_area::GatherArenaBuilder,
                              GatherArenaBuilder, "Gather");

} // namespace exploratron
#endif