#include "exploratron/core/utils/terminal.h"

#include "exploratron/core/utils/logging.h"

#if defined(TERMINAL_WINDOWS_CONSOLE)
#include "exploratron/core/utils/terminal_win.h"
typedef exploratron::terminal::win::Terminal SelectedTerminal;

#elif defined(TERMINAL_LINUX_CONSOLE)
#include "exploratron/core/utils/terminal_linux.h"
typedef exploratron::terminal::linux::Terminal SelectedTerminal;

#elif defined(TERMINAL_WASM_CONSOLE)
#include "exploratron/core/utils/terminal_wasm.h"
typedef exploratron::terminal::wasm::Terminal SelectedTerminal;

#elif defined(TERMINAL_NCURSES_CONSOLE)
#include "exploratron/core/utils/terminal_ncurses.h"
typedef exploratron::terminal::ncurses::Terminal SelectedTerminal;

#elif defined(TERMINAL_SDL_CONSOLE)
#include "exploratron/core/utils/terminal_sdl.h"
typedef exploratron::terminal::sdl::Terminal SelectedTerminal;

#elif defined(TERMINAL_SDL_WASM_CONSOLE)
#include "exploratron/core/utils/terminal_sdl_wasm.h"
typedef exploratron::terminal::sdl_wasm::Terminal SelectedTerminal;


#else
#error("Terminal not set");
#endif

namespace exploratron {
namespace terminal {

SelectedTerminal selected_terminal;

void GetSize(int *width, int *height) {
  selected_terminal.GetSize(width, height);
}

void Initialize() { selected_terminal.Initialize(); }

void Uninitialize() { selected_terminal.Uninitialize(); }

void ClearScreen() { selected_terminal.ClearScreen(); }

void RefreshScreen() { selected_terminal.RefreshScreen(); }

void Sleep(int ms) { selected_terminal.Sleep(ms); }

int GetPressedKey() { return selected_terminal.GetPressedKey(); }

void DrawCharacter(int x, int y, char character, eColor color) {
  selected_terminal.DrawCharacter(x, y, character, color);
}

void DrawSymbol(int x, int y, eSymbol symbol) {
  selected_terminal.DrawSymbol(x, y, symbol);
}

int DrawString(int x, int y, const std::string_view line, eColor color) {
  return selected_terminal.DrawString(x, y, line, color);
}

void DrawTitleBar(int x, int y, int w, std::string text, eColor color) {
  selected_terminal.DrawTitleBar(x, y, w, text, color);
}

}  // namespace terminal
}  // namespace exploratron
