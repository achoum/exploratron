#include "exploratron/core/utils/terminal_interface.h"

#include <cstring>
#include <thread>

namespace exploratron::terminal {

int TerminalInterface::DrawString(int x, int y, const std::string_view line,
                                  eColor color) {
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

void TerminalInterface::DrawTitleBar(int x, int y, int w, std::string text,
                                     eColor color) {
  const int size = text.size();
  const int remaining = w - size - 2;
  const int left = remaining / 2;
  const int right = remaining - left;
  DrawString(x + left + 1, y, text, color);
  for (int i = 0; i < left; i++) {
    DrawSymbol(x + i, y, eSymbol::HORIZONTAL_BAR);
  }
  for (int i = 0; i < right; i++) {
    DrawSymbol(x + left + size + 2 + i, y, eSymbol::HORIZONTAL_BAR);
  }
}

void TextGridTerminalInterface::Initialize() {
  GetSize(&sx, &sy);
  cells.assign(sx * sy, {});
}

void TextGridTerminalInterface::ClearScreen() { cells.assign(sx * sy, {}); }

void TextGridTerminalInterface::DrawCharacter(int x, int y, char character,
                                              eColor color) {
  if (!Inside(x, y)) {
    return;
  }
  auto &c = cell(x, y);
  c.character[0] = character;
  c.character[1] = 0;
  c.color = color;
}

void TextGridTerminalInterface::DrawSymbol(int x, int y, eSymbol symbol) {
  if (!Inside(x, y)) {
    return;
  }

  auto &c = cell(x, y);
  SetCell(symbol, &c);
}

void TerminalInterface::Sleep(int ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

TextCell &TextGridTerminalInterface::cell(int x, int y) {
  DCHECK(Inside(x, y));
  return cells[x + y * sx];
}

bool TextGridTerminalInterface::Inside(int x, int y) {
  return x >= 0 && y >= 0 && x < sx && y < sy;
}



}  // namespace exploratron::terminal