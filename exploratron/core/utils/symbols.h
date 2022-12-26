#ifndef EXPLORATRON_CORE_UTILS_TERMINAL_SYMBOL_H_
#define EXPLORATRON_CORE_UTILS_TERMINAL_SYMBOL_H_

namespace exploratron::terminal {

enum class eColor : int {
  WHITE,
  RED,
  BLUE,
  GREEN,
  GRAY,
  VIOLET,
  YELLOW,
  _NUM_COLOR,
};

int LinuxColor(const eColor color);

enum class eSymbol : int {
  NOTHING,
  WALL,
  SOFT_WALL,
  PLAYER,
  CURSOR,
  HORIZONTAL_BAR,
  BUTTON,
  ARROW_RIGHT,
  ARROW_DOWN,
  ARROW_LEFT,
  ARROW_UP,
  STEEL_WALL,
  AUTOMATIC_DOOR_OPEN,    // -
  AUTOMATIC_DOOR_CLOSED,  // +
  FUNGUS,                 // "
  FUNGUS_TOWER,           // T
  ANT,                    // a
  ANT_QUEEN,              // A
  WATER,                  // ~
  FIRE,                   // ^
  FOOD,                   // f
  EXIT_STAIRS,            // >
  PHEROMONE,              // '
  EXPLOSION,              // *
  BOULDER,                // 'O'
  WIRE,                   // .
  PROXY_SENSOR,           // p
  MESSAGE,                // ?
  TURRET,                 // t
  TURRET_ACTIVE,          // t
  ROBOT,                  // r
  ROBOT_ACTIVE,           // r
  GOLIAT,                 // G
  GOLIAT_ACTIVE,          // G
  EXPLOSIVE_ITEM,         // x
  BARREL,                 // X
  LASER_UP_DOWN,          // |
  LASER_LEFT_RIGHT,       // -
  WORM_HEAD,              // W
  WORM_BODY,              // o
  WORM_TAIL,              // .
  CURSOR_HIT,
  BUTTON_SWITCHED,
  _NUM_SYMBOLS,
};

static constexpr int kSymbolsUtf8NumChars = 4;

inline char* SymbolsUtf8(eSymbol symbol) {
  static char kSymbolsUtf8[(int)(eSymbol::_NUM_SYMBOLS)][kSymbolsUtf8NumChars] =
      {
          " ",       //  NOTHING
          "\u2588",  // Wall
          "\u2593",  // Soft wall
          "@",       // Player
          "\u25C6",  // Cursor
          "\u2550",  // Horizontal bar
          "\u25A3",  // Button
          "\u2BC8",  // right
          "\u2BC6",  // down
          "\u2BC7",  // left
          "\u2BC5",  // up
          "\u2588",  // STEEL_WALL,
          "-",       // AUTOMATIC_DOOR_OPEN,    // -
          "+",       // AUTOMATIC_DOOR_CLOSED,  // +
          "\"",      // FUNGUS,                 // "
          "T",       // FUNGUS_TOWER,           // T
          "a",       // ANT,                    // a
          "A",       // ANT_QUEEN,              // A
          "~",       // WATER,                  // ~
          "^",       // FIRE,                   // ^
          "f",       // FOOD,                   // f
          ">",       // EXIT_STAIRS,            // >
          "'",       // PHEROMONE,              // '
          "*",       // EXPLOSION,              // *
          "O",       // BOULDER,                // 'O'
          ".",       // WIRE,                   // .
          "p",       // PROXY_SENSOR,           // p
          "?",       // MESSAGE,                // ?
          "t",       // TURRET,                 // t
          "t",       // TURRET_ACTIVE,          // t
          "r",       // ROBOT,                  // r
          "r",       // ROBOT_ACTIVE,           // r
          "G",       // GOLIAT,                 // G
          "G",       // GOLIAT_ACTIVE,          // G
          "x",       // EXPLOSIVE_ITEM,         // x
          "X",       // BARREL,                 // X
          "|",       // LASER_UP_DOWN,          // |
          "-",       // LASER_LEFT_RIGHT,       // -
          "W",       // WORM_HEAD,              // W
          "o",       // WORM_BODY,              // o
          ".",       // WORM_TAIL,              // .
          "\u25C6",  // CURSOR_HIT,
          "\u25A3",  // BUTTON_SWITCHED
      };

  return kSymbolsUtf8[(int)symbol];
}

inline eColor SymbolsUtf8Color(eSymbol symbol) {
  static const eColor kSymbolsUtf8Color[(int)eSymbol::_NUM_SYMBOLS] = {
      eColor::WHITE,   // NOTHING
      eColor::GRAY,    // WALL
      eColor::WHITE,   // SOFT_WALL
      eColor::VIOLET,  // PLAYER
      eColor::GREEN,   // CURSOR
      eColor::WHITE,   // HORIZONTAL_BAR
      eColor::WHITE,   // BUTTON
      eColor::GRAY,    // ARROW_RIGHT
      eColor::GRAY,    // ARROW_DOWN
      eColor::GRAY,    // ARROW_LEFT
      eColor::GRAY,    // ARROW_UP
      eColor::BLUE,    // STEEL_WALLS
      eColor::BLUE,    // AUTOMATIC_DOOR_OPEN
      eColor::BLUE,    // AUTOMATIC_DOOR_CLOSED
      eColor::GRAY,    // FUNGUS
      eColor::GRAY,    // FUNGUS_TOWER
      eColor::RED,     // ANT
      eColor::RED,     // ANT_QUEEN
      eColor::BLUE,    // WATER
      eColor::YELLOW,  // FIRE
      eColor::GREEN,   // FOOD
      eColor::GREEN,   // EXIT_STAIRS
      eColor::WHITE,   // PHEROMONE
      eColor::YELLOW,  // EXPLOSION
      eColor::WHITE,   // BOULDER
      eColor::GRAY,    // WIRE
      eColor::WHITE,   // PROXY_SENSOR
      eColor::WHITE,   // MESSAGE
      eColor::BLUE,    // TURRET
      eColor::RED,     // TURRET_ACTIVE
      eColor::BLUE,    // ROBOT
      eColor::RED,     // ROBOT_ACTIVE
      eColor::BLUE,    // GOLIAT
      eColor::RED,     // GOLIAT_ACTIVE
      eColor::GREEN,   // EXPLOSIVE_ITEM
      eColor::GREEN,   // BARREL
      eColor::YELLOW,  // LASER_UP_DOWN
      eColor::YELLOW,  // LASER_LEFT_RIGHT
      eColor::YELLOW,  // WORM_HEAD
      eColor::YELLOW,  // WORM_BODY
      eColor::YELLOW,  // WORM_TAIL
      eColor::RED,     // CURSOR_HIT
      eColor::GRAY,    // BUTTON_SWITCHED
  };

  return kSymbolsUtf8Color[(int)symbol];
}

}  // namespace exploratron::terminal

#endif