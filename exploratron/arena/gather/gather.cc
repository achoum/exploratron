#include "exploratron/arena/gather/gather.h"

#include <assert.h>
#include <stdio.h>

#include <random>
#include <sstream>

#include "absl/strings/str_cat.h"
#include "exploratron/core/abstract_arena.h"
#include "exploratron/core/utils/logging.h"
#include "exploratron/core/utils/maths.h"
#include "exploratron/core/utils/terminal.h"

namespace exploratron {
namespace gather_area {
namespace {

MapDef BuildMapDefinition(const Options &options) {
  int dim = 3;
  if (options.max_coins > 1) {
    dim = 5;
  }
  return MapDef(MatrixShape(7, 7), static_cast<int>(dim));
}

}  // namespace

MapDef GatherArenaBuilder::MapDefinition() const {
  return BuildMapDefinition(options_);
}

GatherArena::GatherArena(
    const std::vector<const AbstractControllerBuilder *> &controller_builders,
    const Options &options) {
  auto seed = std::chrono::system_clock::now().time_since_epoch().count();
  rnd.seed(seed);

  options_ = options;
  map_def = BuildMapDefinition(options_);
  DCHECK_EQ(controller_builders.size(), 1);
  controller_ = controller_builders[0]->Create(map_def);

  input.surouding.Initialize(map_def.shape);

  cells_.assign(options_.width_ * options_.height_, {});

  for (int y = 0; y < options_.height_; y++) {
    cell({0, y}).content = CellContent::WALL;
    cell({options_.width_ - 1, y}).content = CellContent::WALL;
  }

  for (int x = 0; x < options_.width_; x++) {
    cell({x, 0}).content = CellContent::WALL;
    cell({x, options_.height_ - 1}).content = CellContent::WALL;
  }

  if (options_.max_coins > 0) {
    FillCells({1, 1}, {options_.width_ - 2, 1}, CellContent::CHEST);
  }

  SetPlayerPos({options_.width_ / 2, options_.height_ / 2}, false);

  // Random walls.
  FOR_I(20) { AddRandomCell(CellContent::WALL, 2); }

  // Horizontal wall.
  /*
  for (int x = 4; x < options_.width_ - 4; x++) {
    cell({x, options_.height_ / 2 - 1}).content = CellContent::WALL;
  }
  */

  InitRandom(&rnd);
}

void GatherArena::FillCells(Vector2i corner, Vector2i size, CellContent type) {
  for (int x = 0; x < size.x; x++) {
    for (int y = 0; y < size.y; y++) {
      cell({x + corner.x, y + corner.y}).content = type;
    }
  }
}

bool GatherArena::AddRandomCell(CellContent type, int border) {
  std::uniform_int_distribution<int> dist_x(border,
                                            options_.width_ - 2 * border);
  std::uniform_int_distribution<int> dist_y(border,
                                            options_.height_ - 2 * border);
  for (int i = 0; i < 1000; i++) {
    int x = dist_x(rnd);
    int y = dist_y(rnd);
    auto &c = cell({x, y});
    if (c.content == CellContent::NONE) {
      c.content = type;
      return true;
    }
  }
  return false;
}

void GatherArena::SetPlayerPos(Vector2i p, bool check_previous) {
  if (check_previous) {
    auto &previous_cell = cell(player_);
    DCHECK(previous_cell.content == CellContent::CONTROLLER);
    previous_cell.content = CellContent::NONE;
  }
  DCHECK_GE(p.x, 0);
  DCHECK_GE(p.y, 0);
  DCHECK_LT(p.x, options_.width_);
  DCHECK_LT(p.y, options_.height_);

  auto &new_cell = cell(p);
  DCHECK(new_cell.content == CellContent::NONE);
  new_cell.content = CellContent::CONTROLLER;

  player_ = p;
}

void GatherArena::Draw() const {
  for (int y = 0; y < options_.height_; y++) {
    for (int x = 0; x < options_.width_; x++) {
      const auto &c = cell({x, y});
      int display_char = 'e';
      switch (c.content) {
        case CellContent::NONE:
          display_char = ' ';
          break;
        case CellContent::WALL:
          display_char = '#';  // 256 + 0;
          break;
        case CellContent::CONTROLLER:
          display_char = '@';
          break;
        case CellContent::CHEST:
          display_char = 'C';
          break;
        case CellContent::COIN:
          display_char = '$';
          break;
        case CellContent::FOOD:
          display_char = '%';
          break;
        case CellContent::_NUM_TYPES:
          assert(false);
          break;
      }
      terminal::DrawCharacter(x, y, display_char);
    }
  }

  terminal::DrawString(
      0, options_.height_,
      absl::StrCat("score: ", score, " left:", options_.step_left_));
}

void GatherArena::FillInput() {
  int half_sensor = map_def.shape.x / 2;
  for (int y = 0; y < map_def.shape.y; y++) {
    for (int x = 0; x < map_def.shape.x; x++) {
      int value = 0;
      const int cx = player_.x + x - half_sensor;
      const int cy = player_.y + y - half_sensor;
      if (cx >= 0 && cx < options_.width_ && cy >= 0 && cy < options_.height_) {
        const auto &c = cell({cx, cy});
        value = static_cast<int>(c.content);
        if (value >= map_def.num_values) {
          value = 0;
        }
      }
      input.surouding(x, y) = value;
    }
  }
}

bool GatherArena::Step() {
  FillInput();
  const auto control = controller_->Step(input);
  if (control.stop) {
    return false;
  }

  Vector2i dir(control.move);

  if (!dir.IsZero()) {
    Vector2i new_pos = player_ + dir;
    auto &new_pos_cell = cell(new_pos);

    switch (new_pos_cell.content) {
      case CellContent::NONE:
        SetPlayerPos(new_pos);
        break;
      case CellContent::WALL:
      case CellContent::CHEST:
        break;
      case CellContent::CONTROLLER:
        CHECK(false);
        break;
      case CellContent::COIN: {
        Vector2i new_coin_pos = new_pos + dir;
        auto &new_coin_cell = cell(new_coin_pos);
        if (new_coin_cell.content == CellContent::NONE) {
          new_coin_cell.content = CellContent::COIN;
          new_pos_cell.content = CellContent::NONE;
          SetPlayerPos(new_pos);
        } else if (new_coin_cell.content == CellContent::CHEST) {
          score += 10;
          last_score_step = options_.step_left_;
          num_coins--;
          new_pos_cell.content = CellContent::NONE;
          SetPlayerPos(new_pos);
        }
      } break;
      case CellContent::FOOD:
        new_pos_cell.content = CellContent::NONE;
        score += 1;
        last_score_step = options_.step_left_;
        num_food--;
        SetPlayerPos(new_pos);
        break;
      case CellContent::_NUM_TYPES:
        assert(false);
        break;
    }
  }

  if (num_food < options_.max_food) {
    if (AddRandomCell(CellContent::FOOD)) {
      num_food++;
    }
  }

  if (num_coins < options_.max_coins) {
    // if (AddRandomCell(CellContent::COIN, 2)) {
    //   num_coins++;
    // }

    cell({options_.width_ / 2, options_.height_ - 3}).content =
        CellContent::COIN;
    num_coins++;
  }

  if (last_score_step >= 0 && (last_score_step - options_.step_left_ > 100)) {
    return false;
  }

  options_.step_left_--;
  return options_.step_left_ > 0;
}

Scores GatherArena::FinalScore() const { return {static_cast<float>(score)}; }

}  // namespace gather_area
}  // namespace exploratron