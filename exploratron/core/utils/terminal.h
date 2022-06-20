#ifndef EXPLORATRON_CORE_UTILS_TERMINAL_H_
#define EXPLORATRON_CORE_UTILS_TERMINAL_H_

#include <string_view>

namespace exploratron {
namespace terminal {

inline constexpr int kKeyArrowUp = 200;
inline constexpr int kKeyArrowDown = 201;
inline constexpr int kKeyArrowLeft = 202;
inline constexpr int kKeyArrowRight = 203;
inline constexpr int kKeyEscape = 204;
inline constexpr int kKeyReturn = 205;

enum class eColor : int {
#ifdef TERMINAL_WINDOWS_CONSOLE
  WHITE = 15,
  RED = 12,
  BLUE = 9,
  GREEN = 10,
  GRAY = 8,
  VIOLET = 13,
  YELLOW = 14,
#elif defined(TERMINAL_LINUX_CONSOLE) ||                                       \
    defined(TERMINAL_LINUX_CONSOLE_NCURSES) || defined(TERMINAL_WASM_CONSOLE)
  WHITE = 37,
  RED = 31,
  BLUE = 34,
  GREEN = 32,
  GRAY = 90,
  VIOLET = 35,
  YELLOW = 33,
#else
#error "Terminal mode not defined. Compile with --define=terminal=..."
#endif
};

void Initialize();
void Uninitialize();
void ClearScreen();
void RefreshScreen();
int GetPressedKey();
int GetPressedKeyRaw();
int DrawString(int x, int y, const std::string_view line,
               eColor color = eColor::WHITE);
void DrawCharacter(int x, int y, char character, eColor color = eColor::WHITE);
void DrawSymbol(int x, int y, int symbol, eColor color = eColor::WHITE);
void DrawTitleBar(int x, int y, int w, std::string text,
                  eColor color = eColor::WHITE);
void GetSize(int *width, int *height);
void Sleep(int ms);

constexpr char kSymbolBoxDoubleHor = 0xCD;

} // namespace terminal
} // namespace exploratron
#endif