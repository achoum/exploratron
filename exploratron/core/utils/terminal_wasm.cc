#include "exploratron/core/utils/terminal_wasm.h"

#include <emscripten.h>
#include <emscripten/bind.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>

#include "exploratron/core/utils/symbols.h"

namespace exploratron::terminal::wasm {

void Terminal::SetCell(const eSymbol symbol, TextCell *cell) {
  cell->color = SymbolsUtf8Color(symbol);
  cell->SetCharacters(SymbolsUtf8(symbol));
}

int GetPressedKeyRaw() {
  while (true) {
    int key = EM_ASM_INT({ return next_key(); });
    if (key != -1) {
      return key;
    }
    emscripten_sleep(50);
  }
}

void Terminal::Initialize() { TextGridTerminalInterface::Initialize(); }

void Terminal::Uninitialize() { EM_ASM(clear_screen();); }

void Terminal::ClearScreen() { TextGridTerminalInterface::ClearScreen(); }

void Terminal::RefreshScreen() {
  eColor color = eColor::WHITE;
  std::stringstream ss;
  for (int y = 0; y < sy; y++) {
    for (int x = 0; x < sx; x++) {
      auto &c = cell(x, y);
      if (c.color != color) {
        color = c.color;
        ss << "\x1B" << (char)LinuxColor(color);
      }
      ss << c.character;
    }
    if (y < sy - 1) {
      ss << "</br>";
    }
  }
  ss << "\x1B" << (char)LinuxColor(eColor::WHITE) << "\n";
  std::cout << ss.str() << std::flush;

  EM_ASM(refresh_screen(););
}

void Terminal::GetSize(int *width, int *height) {
  *width = 120;
  *height = 40;
}

void Terminal::Sleep(int ms) { emscripten_sleep(ms); }

int Terminal::GetPressedKey() {
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
}

}  // namespace exploratron::terminal::wasm