#include "exploratron/core/utils/terminal_ncurses.h"

#include <curses.h>

namespace exploratron::terminal::ncurses {

char symbol_to_text[(int)(eSymbol::_NUM_SYMBOLS)][TextCell::kNumChars] = {
    "#",    // Wall
    "&",    // Soft wall
    "@",    // Player
    "o",    // Cursor
    "=",    // Horizontal bar
    "¤",    // Button
    "\26",  // right
    "\25",  // down
    "\27",  // left
    "\24",  // up
};

void Terminal::SetCell(const eSymbol symbol, TextCell *cell) {
  cell->color = SymbolColor(symbol);
  cell->SetCharacters(symbol_to_text[(int)symbol]);
}

int GetPressedKeyRaw() { return getch(); }

void Terminal::Initialize() {
  TextGridTerminalInterface::Initialize();
  setlocale(LC_ALL, "");
  initscr();
  // timeout(-1);
  noecho();
  // raw();
}

void Terminal::Uninitialize() {
  endwin();
  std::cout << "Endwin called" << std::endl;
}

void Terminal::ClearScreen() {
  TextGridTerminalInterface::ClearScreen();
  erase();
}

void Terminal::RefreshScreen() {
  // mvaddch(y, x, character);
  refresh();
}

void Terminal::GetSize(int *width, int *height) {
  getmaxyx(stdscr, *height, *width);
}

int Terminal::GetPressedKey() { return GetPressedKeyRaw(); }

}  // namespace exploratron::terminal::ncurses