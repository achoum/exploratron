#ifndef EXPLORATRON_CORE_UTILS_TERMINAL_INTERFACE_H_
#define EXPLORATRON_CORE_UTILS_TERMINAL_INTERFACE_H_

#include <cstring>
#include <string_view>
#include <vector>

#include "exploratron/core/utils/logging.h"
#include "exploratron/core/utils/symbols.h"

namespace exploratron::terminal {

inline constexpr int kKeyArrowUp = 200;
inline constexpr int kKeyArrowDown = 201;
inline constexpr int kKeyArrowLeft = 202;
inline constexpr int kKeyArrowRight = 203;
inline constexpr int kKeyEscape = 204;
inline constexpr int kKeyReturn = 205;

// constexpr char kSymbolBoxDoubleHor = 0xCD;

class TerminalInterface {
 public:
  virtual void Initialize() = 0;
  virtual void Uninitialize() = 0;
  virtual void ClearScreen() = 0;
  virtual void RefreshScreen() = 0;
  virtual void GetSize(int *width, int *height) = 0;
  virtual void Sleep(int ms);
  virtual int GetPressedKey() = 0;

  virtual void DrawCharacter(int x, int y, char character, eColor color) = 0;
  virtual void DrawSymbol(int x, int y, eSymbol symbol) = 0;

  virtual int DrawString(int x, int y, const std::string_view line,
                         eColor color);
  virtual void DrawTitleBar(int x, int y, int w, std::string text,
                            eColor color);
};

struct TextCell {
  eColor color = eColor::WHITE;
  char character[kSymbolsUtf8NumChars];

  TextCell() { Reset(); }

  TextCell(char character[kSymbolsUtf8NumChars], eColor color) {
    std::memcpy(character, this->character, kSymbolsUtf8NumChars);
    this->color = color;
  }

  void Copy(const TextCell &src) {
    std::memcpy(character, src.character, kSymbolsUtf8NumChars);
    color = src.color;
  }

  void SetCharacters(char src[kSymbolsUtf8NumChars]) {
    std::memcpy(character, src, kSymbolsUtf8NumChars);
  }

  void Reset() {
    character[0] = ' ';
    character[1] = 0;
  }
};

struct TextGridTerminalInterface : public TerminalInterface {
  void Initialize() override;
  void ClearScreen() override;
  void DrawCharacter(int x, int y, char character, eColor color) override;
  void DrawSymbol(int x, int y, eSymbol symbol) override;
  virtual void SetCell(eSymbol symbol, TextCell *cell) = 0;

  void Initialize(int sx, int sy);
  bool Inside(int x, int y);
  TextCell &cell(int x, int y);

  std::vector<TextCell> cells;
  int sx = -1;
  int sy = -1;
};

}  // namespace exploratron::terminal

#endif