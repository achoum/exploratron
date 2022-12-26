#include "exploratron/core/utils/terminal_win.h"

#include <conio.h>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace exploratron::terminal::win {
namespace {

// https://en.wikipedia.org/wiki/Code_page_437#Characters
TextCell textcells[(int)(eSymbol::_NUM_SYMBOLS)] = {
    TextCell("\xDB", eColor::GRAY),  // Wall
    TextCell("\xB1", eColor::GRAY),  // Soft wall
    TextCell("@", eColor::GRAY),     // Player
    TextCell("\x04", eColor::GRAY),  // Cursor
    TextCell("\xCD", eColor::GRAY),  // Horizontal bar
    TextCell("\xC1", eColor::GRAY),  // Button
    TextCell("\x10", eColor::GRAY),  // right
    TextCell("\x1F", eColor::GRAY),  // down
    TextCell("\x11", eColor::GRAY),  // left
    TextCell("\x1E", eColor::GRAY),  // up
};

int WinColor(const eColor color) {
  switch (color) {
    case eColor::WHITE:
      return 15;
    case eColor::RED:
      return 12;
    case eColor::BLUE:
      return 9;
    case eColor::GREEN:
      return 10;
    case eColor::GRAY:
      return 8;
    case eColor::VIOLET:
      return 13;
    case eColor::YELLOW:
      return 14;
  }
}

int GetPressedKeyRaw() { return _getch(); }

}  // namespace

void Terminal::SetCell(const eSymbol symbol, TextCell *cell) {
  cell->Copy(textcells[(int)symbol]);
}

void Terminal::Initialize() { TextGridTerminalInterface::Initialize(); }

void Terminal::Uninitialize() {}

void Terminal::ClearScreen() { TextGridTerminalInterface::ClearScreen(); }

void Terminal::RefreshScreen() {
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
}

void Terminal::GetSize(int *width, int *height) {
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
  *width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
  *height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
}

int Terminal::GetPressedKey() {
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
}

}  // namespace exploratron::terminal::win