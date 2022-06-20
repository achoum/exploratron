#include "exploratron/core/utils/terminal.h"

#include "exploratron/core/utils/logging.h"

// This program will clear the screen and print a message.
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <vector>

constexpr int kNumChars = 4;

#ifndef TERMINAL_WASM_CONSOLE
#include <chrono>
#include <thread>
#endif

#ifdef TERMINAL_WINDOWS_CONSOLE
#include <conio.h>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// https://en.wikipedia.org/wiki/Code_page_437#Characters
char symbols[][kNumChars] = {
    "\xDB", // Wall
    "\xB1", // Soft wall
    "@",    // Player // \x02
    "\x04", // Cursor
    "\xCD", // Horizontal bar
    "¤",    // Button
    "\26",  // right
    "\25",  // down
    "\27",  // left
    "\24"   // up
};

#elif defined(TERMINAL_LINUX_CONSOLE) || defined(TERMINAL_WASM_CONSOLE)

#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(TERMINAL_LINUX_CONSOLE)
#include <sys/ioctl.h>
#include <term.h>
#include <termios.h>
#include <unistd.h>

#elif defined(TERMINAL_WASM_CONSOLE)
#include <emscripten.h>
#include <emscripten/bind.h>

#endif

// Support UTF8
char symbols[][kNumChars] = {
    "\u2588", // Wall
    "\u2593", // Soft wall
    "@",      // Player
    "\u25C6", // Cursor
    "\u2550", // Horizontal bar
    "\u25A3", // Button
    "\u2BC8", // right
    "\u2BC6", // down
    "\u2BC7", // left
    "\u2BC5"  // up
};

#elif defined(TERMINAL_LINUX_CONSOLE_NCURSES)
#include <curses.h>

char symbols[][kNumChars] = {
    "#",   // Wall
    "&",   // Soft wall
    "@",   // Player
    "o",   // Cursor
    "=",   // Horizontal bar
    "¤",   // Button
    "\26", // right
    "\25", // down
    "\27", // left
    "\24"  // up
};

#else
#error "Terminal mode not defined. Compile with --define=terminal=..."
#endif

namespace exploratron {
namespace terminal {

struct TextScreen {
  struct Cell {
    eColor color = eColor::WHITE;
    char character[kNumChars];

    Cell() { Reset(); }
    void Reset() {
      character[0] = ' ';
      character[1] = 0;
    }
  };

  std::vector<Cell> cells;
  int sx = -1;
  int sy = -1;

  void Initialize(int sx, int sy) {
    if (this->sx == sx && this->sy == sy) {
      for (auto &c : cells) {
        c.Reset();
      }
    } else {
      this->sx = sx;
      this->sy = sy;
      cells.assign(sx * sy, {});
    }
  }

  bool Inside(int x, int y) { return x >= 0 && y >= 0 && x < sx && y < sy; }

  Cell &cell(int x, int y) {
    DCHECK(Inside(x, y));
    return cells[x + y * sx];
  }
};

struct Terminal {
  bool initialized = false;
#if defined(TERMINAL_WINDOWS_CONSOLE) || defined(TERMINAL_LINUX_CONSOLE) ||    \
    defined(TERMINAL_WASM_CONSOLE)
  TextScreen screen;
#endif
};
Terminal global_terminal;

void GetSize(int *width, int *height) {
#ifdef TERMINAL_WINDOWS_CONSOLE
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
  *width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
  *height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#elif defined(TERMINAL_LINUX_CONSOLE)

  struct winsize size;
  if (ioctl(0, TIOCSWINSZ, (char *)&size) < 0) {
    abort();
  }
  *width = size.ws_col;
  *height = size.ws_row;

  if (*width > 8000) {
    // ioctl does not work on WSL.
    *width = 120;
    *height = 40;
  }

#elif defined(TERMINAL_LINUX_CONSOLE_NCURSES)
  getmaxyx(stdscr, *height, *width);

#elif defined(TERMINAL_WASM_CONSOLE)
  *width = 120;
  *height = 40;

#endif
}

void Initialize() {
  DCHECK(!global_terminal.initialized);
  global_terminal.initialized = true;

#if defined(TERMINAL_WINDOWS_CONSOLE) || defined(TERMINAL_LINUX_CONSOLE) ||    \
    defined(TERMINAL_WASM_CONSOLE)
  int width;
  int height;
  GetSize(&width, &height);
  global_terminal.screen.Initialize(width, height);

#ifdef TERMINAL_LINUX_CONSOLE
  system("clear");
#endif

#elif defined(TERMINAL_LINUX_CONSOLE_NCURSES)
  setlocale(LC_ALL, "");
  initscr();
  // timeout(-1);
  noecho();
  // raw();
#endif
}

void Uninitialize() {
#if defined(TERMINAL_WINDOWS_CONSOLE)
  global_terminal = {};
#elif defined(TERMINAL_LINUX_CONSOLE)
  global_terminal = {};
  system("clear");
  system("reset");
#elif defined(TERMINAL_WASM_CONSOLE)
  global_terminal = {};
  EM_ASM(clear_screen(););
#elif defined(TERMINAL_LINUX_CONSOLE_NCURSES)
  endwin();
  std::cout << "Endwin called" << std::endl;
#endif
}

void ClearScreen() {
#if defined(TERMINAL_WINDOWS_CONSOLE) || defined(TERMINAL_LINUX_CONSOLE)
  int width;
  int height;
  GetSize(&width, &height);
  global_terminal.screen.Initialize(width, height);
#elif defined(TERMINAL_WASM_CONSOLE)
  int width;
  int height;
  GetSize(&width, &height);
  global_terminal.screen.Initialize(width, height);
  EM_ASM(clear_screen(););
#elif defined(TERMINAL_LINUX_CONSOLE_NCURSES)
  erase();
#endif
}

void RefreshScreen() {
#ifdef TERMINAL_WINDOWS_CONSOLE
  HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
  COORD c;
  c.X = 0;
  c.Y = 0;
  SetConsoleCursorPosition(h, c);
  eColor color = eColor::WHITE;

  std::stringstream ss;
  for (int y = 0; y < global_terminal.screen.sy; y++) {
    for (int x = 0; x < global_terminal.screen.sx; x++) {
      auto &cell = global_terminal.screen.cell(x, y);
      if (cell.color != color) {
        std::cout << ss.str();
        ss.str("");
        color = cell.color;
        SetConsoleTextAttribute(h, (WORD)color);
      }
      ss << cell.character;
    }
    if (y < global_terminal.screen.sy - 1) {
      ss << "\n";
    }
  }
  std::cout << ss.str() << std::flush;
  SetConsoleTextAttribute(h, (WORD)eColor::WHITE);

#elif defined(TERMINAL_LINUX_CONSOLE)
  printf("%c[%d;%df", 0x1B, 0, 0);
  eColor color = eColor::WHITE;
  std::stringstream ss;
  for (int y = 0; y < global_terminal.screen.sy; y++) {
    for (int x = 0; x < global_terminal.screen.sx; x++) {
      auto &cell = global_terminal.screen.cell(x, y);
      if (cell.color != color) {
        color = cell.color;
        ss << "\x1B[" << (int)color << "m";
      }
      ss << cell.character;
    }
    if (y < global_terminal.screen.sy - 1) {
      ss << "\n";
    }
  }
  ss << "\x1B[" << (int)eColor::WHITE << "m";
  std::cout << ss.str() << std::flush;

#elif defined(TERMINAL_WASM_CONSOLE)
  eColor color = eColor::WHITE;
  std::stringstream ss;
  for (int y = 0; y < global_terminal.screen.sy; y++) {
    for (int x = 0; x < global_terminal.screen.sx; x++) {
      auto &cell = global_terminal.screen.cell(x, y);
      if (cell.color != color) {
        color = cell.color;
        ss << "\x1B" << (char)color;
      }
      ss << cell.character;
    }
    if (y < global_terminal.screen.sy - 1) {
      ss << "</br>";
    }
  }
  ss << "\x1B" << (char)eColor::WHITE << "\n";
  std::cout << ss.str() << std::flush;

  EM_ASM(refresh_screen(););
#elif defined(TERMINAL_LINUX_CONSOLE_NCURSES)
  refresh();
#endif
}

void Sleep(int ms) {
#if defined(TERMINAL_WASM_CONSOLE)
  emscripten_sleep(ms);
#else
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
#endif
}

int GetPressedKeyRaw() {
#ifdef TERMINAL_WINDOWS_CONSOLE
  return _getch();

#elif defined(TERMINAL_LINUX_CONSOLE)
  int character;
  struct termios orig_term_attr;
  struct termios new_term_attr;
  tcgetattr(fileno(stdin), &orig_term_attr);
  memcpy(&new_term_attr, &orig_term_attr, sizeof(struct termios));
  new_term_attr.c_lflag &= ~(ECHO | ICANON);
  tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);
  character = fgetc(stdin);
  tcsetattr(fileno(stdin), TCSANOW, &orig_term_attr);
  return character;

#elif defined(TERMINAL_WASM_CONSOLE)
  while (true) {
    int key = EM_ASM_INT({ return next_key(); });
    if (key != -1) {
      return key;
    }
    emscripten_sleep(50);
  }

#elif defined(TERMINAL_LINUX_CONSOLE_NCURSES)
  return getch();
#endif
}

int GetPressedKey() {

#ifdef TERMINAL_WINDOWS_CONSOLE
  int key = GetPressedKeyRaw();
  if (key == 224) {
    key = GetPressedKeyRaw();
    switch (key) {
    case 72:
      return kKeyArrowUp;
    case 75:
      return kKeyArrowLeft;
    case 80:
      return kKeyArrowDown;
    case 77:
      return kKeyArrowRight;
    }
  } else {
    switch (key) {
    case 27:
      return kKeyEscape;
    case 13:
      return kKeyReturn;
    }
    return key;
  }

#elif defined(TERMINAL_LINUX_CONSOLE)
  int key = GetPressedKeyRaw();
  if (key == 27) {
    key = GetPressedKeyRaw();
    if (key != 91) {
      return key;
    }
    key = GetPressedKeyRaw();
    switch (key) {
    case 65:
      return kKeyArrowUp;
    case 68:
      return kKeyArrowLeft;
    case 66:
      return kKeyArrowDown;
    case 67:
      return kKeyArrowRight;
    }
  } else if (key == '\n' || key == '\r') {
    return kKeyReturn;
  }
  return key;

#elif defined(TERMINAL_WASM_CONSOLE)
  int key = GetPressedKeyRaw();
  switch (key) {
  case 38:
    return kKeyArrowUp;
  case 37:
    return kKeyArrowLeft;
  case 40:
    return kKeyArrowDown;
  case 39:
    return kKeyArrowRight;
  case 27:
    return kKeyEscape;
  case 13:
    return kKeyReturn;
  }
  return key;

#elif defined(TERMINAL_LINUX_CONSOLE_NCURSES)
  return GetPressedKeyRaw();

#else

#error "Not implemented"
#endif
}

void DrawCharacter(int x, int y, char character, eColor color) {
#ifdef TERMINAL_WINDOWS_CONSOLE
  if (!global_terminal.screen.Inside(x, y)) {
    return;
  }
  auto &cell = global_terminal.screen.cell(x, y);
  cell.character[0] = character;
  cell.character[1] = 0;
  cell.color = color;

#elif defined(TERMINAL_LINUX_CONSOLE) || defined(TERMINAL_WASM_CONSOLE)
  if (!global_terminal.screen.Inside(x, y)) {
    return;
  }
  auto &cell = global_terminal.screen.cell(x, y);
  cell.character[0] = character;
  cell.character[1] = 0;
  cell.color = color;

#elif defined(TERMINAL_LINUX_CONSOLE_NCURSES)
  mvaddch(y, x, character);
#endif
}

void DrawSymbol(int x, int y, int symbol, eColor color) {
  if (symbol < 256) {
    DrawCharacter(x, y, (char)symbol, color);
    return;
  }
  symbol -= 256;

#if defined(TERMINAL_WINDOWS_CONSOLE) || defined(TERMINAL_LINUX_CONSOLE) ||    \
    defined(TERMINAL_WASM_CONSOLE)
  if (!global_terminal.screen.Inside(x, y)) {
    return;
  }
  auto &cell = global_terminal.screen.cell(x, y);
  std::memcpy(cell.character, symbols[symbol], kNumChars);
  cell.color = color;
#elif defined(TERMINAL_LINUX_CONSOLE_NCURSES)
  mvaddstr(y, x, symbols[symbol]);
#endif
}

int DrawString(int x, int y, const std::string_view line, eColor color) {
  auto save_x = x;
  for (auto character : line) {
    if (character == '\n') {
      y++;
      x = save_x;
      continue;
    }
    DrawCharacter(x++, y, character, color);
  }
  return line.size();
}

void DrawTitleBar(int x, int y, int w, std::string text, eColor color) {
  const int size = text.size();
  const int remaining = w - size - 2;
  const int left = remaining / 2;
  const int right = remaining - left;
  DrawString(x + left + 1, y, text, color);
  for (int i = 0; i < left; i++) {
    DrawSymbol(x + i, y, 256 + 4);
  }
  for (int i = 0; i < right; i++) {
    DrawSymbol(x + left + size + 2 + i, y, 256 + 4);
  }
}

} // namespace terminal
} // namespace exploratron
