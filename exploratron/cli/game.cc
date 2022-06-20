#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <string>

#include "absl/debugging/failure_signal_handler.h"
#include "absl/debugging/symbolize.h"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_split.h"
#include "exploratron/arena/common_game/common_game.h"
#include "exploratron/controller/buffer/buffer.h"
#include "exploratron/core/abstract_arena.h"
#include "exploratron/core/abstract_controller.h"
#include "exploratron/core/abstract_game_area.h"
#include "exploratron/core/evaluate.h"
#include "exploratron/core/utils/terminal.h"

namespace exploratron {

constexpr char kVersion[] = "0.1";

struct MapInfo {
  std::string path;
  std::string format;
  std::string name;
  int idx;
};

enum class eControls {
  UNKNOWN,
  NONE,
  WAIT,
  RIGHT,
  DOWN,
  LEFT,
  UP,
  INTERACTION,
  ACTION,
  INVENTORY,
  MAP,
  LOGS,
  QUIT,
  ENTER,
  HELP,
};

eDirection ControlToDirection(const eControls a) {
  switch (a) {
  case eControls::WAIT:
    return eDirection::NONE;
  case eControls::UP:
    return eDirection::UP;
  case eControls::LEFT:
    return eDirection::LEFT;
  case eControls::DOWN:
    return eDirection::DOWN;
  case eControls::RIGHT:
    return eDirection::RIGHT;
  default:
    return eDirection::NONE;
  }
}

eControls KeyToControl(int key) {
  switch (key) {
  case terminal::kKeyArrowUp:
    return eControls::UP;

  case terminal::kKeyArrowLeft:
    return eControls::LEFT;

  case terminal::kKeyArrowDown:
    return eControls::DOWN;

  case terminal::kKeyArrowRight:
    return eControls::RIGHT;

  case ' ':
    return eControls::WAIT;

  case 'a':
    return eControls::ACTION;

  case '\n':
    return eControls::ENTER;

  case 'h':
    return eControls::HELP;

  case terminal::kKeyEscape:
  case 'Q':
  case 'q':
    return eControls::QUIT;

  default:
    return eControls::UNKNOWN;
  }
}

std::optional<abstract_game_area::Action>
SelectMagic(abstract_game_area::AbstractGameArena *game,
            std::shared_ptr<abstract_game_area::Entity> controlled) {
  const auto actions = controlled->AvailableMagics();

  terminal::ClearScreen();
  int ui_x = 0;
  int ui_y = 0;

  terminal::DrawString(ui_x, ui_y++, "Select action");
  terminal::DrawString(ui_x, ui_y++, "=============");
  for (const auto &a : actions) {
    terminal::DrawString(ui_x + 1, ui_y++,
                         absl::StrFormat(" %c) %s", a.shortcut, a.label));
  }
  terminal::DrawString(ui_x + 1, ui_y++, " q) cancel");
  terminal::DrawString(ui_x, ui_y++, "=============");
  terminal::RefreshScreen();

  while (true) {
    const auto key = terminal::GetPressedKey();
    if (key == 'q' || key == terminal::kKeyEscape) {
      return {};
    }
    for (const auto &a : actions) {
      if (a.shortcut == key) {
        return a;
      }
    }
  }
}

std::optional<Vector2i>
SelectTarget(abstract_game_area::AbstractGameArena *game, Vector2i pos,
             int non_passable_tag, int max_dist) {
  Vector2i target = pos;
  target.x++;
  while (true) {
    // Display
    terminal::ClearScreen();
    game->Draw();
    game->map().IterateLine(pos, target, [&](const Vector2i &p) {
      if (p != pos) {
        terminal::DrawSymbol(p.x, p.y, 256 + 3, terminal::eColor::RED);
      }
      return true;
    });
    terminal::RefreshScreen();

    // Get next control
    eControls control;
    bool has_action = false;
    while (true) {
      const auto key = terminal::GetPressedKey();
      control = KeyToControl(key);

      switch (control) {
      case eControls::QUIT:
        return {};

      case eControls::ACTION:
      case eControls::ENTER:
        return target;
        break;

      case eControls::UP:
      case eControls::LEFT:
      case eControls::DOWN:
      case eControls::RIGHT:
        auto candidate = target + Vector2i(ControlToDirection(control));
        if (game->map().Contains(candidate) &&
            (pos - candidate).MaxLength() <= max_dist) {
          target = candidate;
          has_action = true;
        }
        break;
      }
      if (has_action) {
        break;
      }
    }
  }
}

void ShowHelp() {
  terminal::ClearScreen();
  int ui_x = 0;
  int ui_y = 0;

  terminal::DrawString(ui_x, ui_y++, "Help");
  terminal::DrawString(ui_x, ui_y++, "=============");

  struct Item {
    common_game::DisplaySymbol symbol;
    std::shared_ptr<common_game::Entity> entity;
  };

  std::vector<Item> display_entities;
  for (auto entity_def : abstract_game_area::global_registered_entities) {
    auto entity = entity_def.second.builder();
    auto display = entity->Display();
    if (!display.help_) {
      continue;
    }
    if (display.symbol_ == ' ') {
      continue;
    }
    display_entities.push_back({display, entity});
  }

  std::sort(display_entities.begin(), display_entities.end(),
            [](const Item &a, const Item &b) {
              return a.symbol.priotity_ > b.symbol.priotity_;
            });

  for (const auto &display : display_entities) {

    terminal::DrawSymbol(ui_x, ui_y, display.symbol.symbol_,
                         display.symbol.color);
    terminal::DrawString(ui_x + 1, ui_y++,
                         absl::StrFormat(" %s. %s (%d)", display.entity->Name(),
                                         display.entity->status(),
                                         display.symbol.priotity_));
  }

  terminal::DrawString(ui_x, ui_y++, "=============");
  terminal::DrawString(ui_x, ui_y++, "Press return to continue");
  terminal::RefreshScreen();
  while (terminal::GetPressedKey() != terminal::kKeyReturn) {
  }
}

void PrintFinalScreen(abstract_game_area::AbstractGameArena *game) {
  terminal::ClearScreen();
  int ui_x = 0;
  int ui_y = 0;

  int tmp = ui_x;
  tmp += terminal::DrawString(tmp, ui_y, "You reached the exit in ");
  tmp += terminal::DrawString(tmp, ui_y, absl::StrCat(game->map().time()),
                              terminal::eColor::RED);
  tmp += terminal::DrawString(tmp, ui_y, " steps");

  auto all_controlled = game->map().ControlledEntities();
  if (!all_controlled.empty()) {
    tmp += terminal::DrawString(tmp, ui_y, " and with ");
    tmp += terminal::DrawString(tmp, ui_y,
                                absl::StrCat(all_controlled.front()->hp()),
                                terminal::eColor::GREEN);
    tmp += terminal::DrawString(tmp, ui_y, " hp.");
  } else {
    tmp += terminal::DrawString(tmp, ui_y, ".");
  }
  /*
  ui_y++;
  ui_y++;
  terminal::DrawString(ui_x, ui_y++, "Can you finish faster and with more hp?");
  */
  ui_y++;
  ui_y++;
  terminal::DrawString(ui_x, ui_y++, "=============");
  terminal::DrawString(ui_x, ui_y++, "Press return to continue");
  terminal::RefreshScreen();
  while (terminal::GetPressedKey() != terminal::kKeyReturn) {
  }
}

void RunArea(const MapInfo &map) {
  const auto arena_builder = AbstractArenaBuilderRegisterer::Create("Ant");
  buffer::BufferInput buffer_input;
  const auto controller_builder =
      buffer::BufferControllerBuilder(&buffer_input);

  arena_builder->SetParameter(map.path);
  auto area = arena_builder->Create({&controller_builder});

  // Ensure that the arena is a game arena.
  auto *game =
      dynamic_cast<abstract_game_area::AbstractGameArena *>(area.get());
  DCHECK(game);

  // Run area.
  while (true) {
    // Display
    terminal::ClearScreen();
    area->Draw();
    terminal::RefreshScreen();

    // Get next control
    eControls control;
    while (true) {
      const auto key = terminal::GetPressedKey();
      control = KeyToControl(key);
      if (control == eControls::UNKNOWN) {
        continue;
      }
      break;
    }

    // Get next action
    Output action;
    bool has_action = false;
    switch (control) {
    case eControls::QUIT:
      return;

    case eControls::WAIT:
    case eControls::UP:
    case eControls::LEFT:
    case eControls::DOWN:
    case eControls::RIGHT:
      action.action = eAction::MOVE;
      action.move = ControlToDirection(control);
      has_action = true;
      break;

    case eControls::ACTION: {
      // Link to player entity.
      auto all_controlled = game->map().ControlledEntities();
      if (all_controlled.empty()) {
        break;
      }
      auto controlled = all_controlled.front();
      const auto magic_action = SelectMagic(game, controlled);
      if (!magic_action.has_value()) {
        break; // Cancel
      }

      if (magic_action.value().target_radius > 0) {
        auto target = SelectTarget(game, controlled->position(),
                                   common_game::Tag::NON_PASSABLE,
                                   magic_action.value().target_radius);
        if (!target.has_value()) {
          break; // Cancel
        }
        action.action = eAction::MAGIC;
        action.magic_idx = magic_action.value().idx;
        action.target = target.value();
        has_action = true;
        break;

      } else {
        action.action = eAction::MAGIC;
        action.magic_idx = magic_action.value().idx;
        has_action = true;
        break;
      }
    } break;

    case eControls::HELP:
      ShowHelp();
      break;

    case eControls::INTERACTION:
    case eControls::INVENTORY:
    case eControls::MAP:
    case eControls::LOGS:
      // TODO
      break;
    }

    // Run action
    if (has_action) {
      buffer_input.Add(action);
      if (!area->Step()) {
        PrintFinalScreen(game);
        break;
      }
    }
  }

  // Final display
  terminal::ClearScreen();
  area->Draw();
  terminal::RefreshScreen();
}

std::vector<MapInfo> ListMaps() {
  std::vector<MapInfo> maps;
  std::ifstream file("exploratron/assets/map/list.txt");
  std::string line;
  int idx = 0;
  while (std::getline(file, line)) {
    std::vector<std::string> items = absl::StrSplit(line, "\t");
    CHECK_EQ(items.size(), 3);
    maps.push_back(
        {.path = items[0], .format = items[1], .name = items[2], .idx = idx++});
  }
  return maps;
}

std::optional<MapInfo> SelectMap(int selection) {
  terminal::RefreshScreen();
  const auto maps = ListMaps();
  while (true) {
    terminal::ClearScreen();

    int ui_x = 1;
    int ui_y = 1;

    auto print_item = [&](int item_idx, std::string text) {
      terminal::DrawString(ui_x, ui_y, absl::StrCat("[ ] ", text));
      if (item_idx == selection) {
        terminal::DrawSymbol(ui_x + 1, ui_y, 256 + 3, terminal::eColor::RED);
      }
      ui_y++;
    };

    terminal::DrawString(ui_x, ui_y, "Exploratron", terminal::eColor::GREEN);
    terminal::DrawString(ui_x + 22, ui_y++, absl::StrCat("v", kVersion),
                         terminal::eColor::GRAY);
    terminal::DrawString(ui_x, ui_y++, "  by Mathieu Guillame-Bert",
                         terminal::eColor::GRAY);
    int tmp = ui_x;
    tmp +=
        terminal::DrawString(tmp, ui_y, "  https://", terminal::eColor::GRAY);
    tmp += terminal::DrawString(tmp, ui_y, "achoum.github.io",
                                terminal::eColor::BLUE);
    ui_y++;

    ui_y++;

    terminal::DrawString(
        ui_x, ui_y++,
        "Reach the exit (>) quickly and without loosing health (hp).");
    terminal::DrawString(ui_x, ui_y++,
                         "Use your surouding and actions (a) to survive.");
    ui_y++;

    terminal::DrawString(ui_x, ui_y++, "Select a world:");
    ui_y++;

    for (int i = 0; i < maps.size(); i++) {
      const auto &map = maps[i];
      print_item(i, absl::StrCat(map.name)); //, " [", map.path, "]"));
    }
    print_item(maps.size(), "Quit");

    terminal::RefreshScreen();
    auto key = terminal::GetPressedKey();
    switch (key) {
    case terminal::kKeyArrowUp:
      if (selection > 0) {
        selection--;
      }
      break;
    case terminal::kKeyArrowLeft:
      break;
    case terminal::kKeyArrowDown:
      if (selection < maps.size()) {
        selection++;
      }
      break;
    case terminal::kKeyArrowRight:
      break;
    case terminal::kKeyReturn: {
      if (selection == maps.size()) {
        return {};
      } else {
        return maps[selection];
      }
    } break;
    case terminal::kKeyEscape:
      return {};
    }
  }
}

void TestKey() {
  while (true) {
    int a = terminal::GetPressedKeyRaw();
    std::cout << a << std::endl;
    if (a == 3) {
      break;
    }
  }
}

void Intro() {
  int width;
  int height;
  terminal::GetSize(&width, &height);
  width = 120;
  height = 40;
  float character_ratio = 12.f / 8.f;
  terminal::ClearScreen();
  for (int frame = 0; frame < 80; frame += 1) {
    terminal::ClearScreen();

    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        int dist2 = (x - width / 2) * (x - width / 2) +
                    (y - height / 2) * (y - height / 2) * character_ratio *
                        character_ratio;
        int d = frame;
        if (dist2 > d * d) {
          terminal::DrawSymbol(x, y, 256 + 0, terminal::eColor::RED);
        }
      }
    }

    terminal::RefreshScreen();
    terminal::Sleep(10);
  }
  terminal::ClearScreen();
}

void Main() {
// TestKey();
#ifndef SKIP_INTRO
  Intro(); // Does not always work on LINUX on Windows.
#endif

  int selected_map = 0;

  while (true) {
    // Select map
    auto map_info = SelectMap(selected_map);
    if (!map_info.has_value()) {
      std::cout << std::endl << "Bye" << std::endl;
      return;
    }
    selected_map = map_info.value().idx;

    // Play map
    RunArea(map_info.value());
  }
}

} // namespace exploratron

int main(int argc, char **argv) {

#ifdef SHOW_STACK_TRACE_IF_ABORT
  absl::InitializeSymbolizer(argv[0]);
  absl::FailureSignalHandlerOptions options;
  absl::InstallFailureSignalHandler(options);
#endif

  absl::ParseCommandLine(argc, argv);
  exploratron::terminal::Initialize();
  exploratron::Main();
  exploratron::terminal::Uninitialize();
}