#include "exploratron/core/utils/terminal_linux.h"

#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <term.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <sstream>
#include <thread>

#include "exploratron/core/utils/symbols.h"

namespace exploratron::terminal::linux {

void Terminal::SetCell(const eSymbol symbol, TextCell *cell) {
  cell->color = SymbolsUtf8Color(symbol);
  cell->SetCharacters(SymbolsUtf8(symbol));
}

int GetPressedKeyRaw() {
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
}

void Terminal::Initialize() {
  TextGridTerminalInterface::Initialize();
  system("clear");
}

void Terminal::Uninitialize() {
  system("clear");
  system("reset");
}

void Terminal::ClearScreen() { TextGridTerminalInterface::ClearScreen(); }

void Terminal::RefreshScreen() {
  printf("%c[%d;%df", 0x1B, 0, 0);
  eColor color = eColor::WHITE;
  std::stringstream ss;
  for (int y = 0; y < sy; y++) {
    for (int x = 0; x < sx; x++) {
      auto &c = cell(x, y);
      if (c.color != color) {
        color = c.color;
        ss << "\x1B[" << LinuxColor(color) << "m";
      }
      ss << c.character;
    }
    if (y < sy - 1) {
      ss << "\n";
    }
  }
  ss << "\x1B[" << LinuxColor(eColor::WHITE) << "m";
  std::cout << ss.str() << std::flush;
}

void Terminal::GetSize(int *width, int *height) {
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
}

int Terminal::GetPressedKey() {
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
}

}  // namespace exploratron::terminal::linux