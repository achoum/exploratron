#ifndef EXPLORATRON_CORE_UTILS_TERMINAL_H_
#define EXPLORATRON_CORE_UTILS_TERMINAL_H_

#include <string_view>

#include "exploratron/core/utils/terminal_interface.h"

namespace exploratron {
namespace terminal {

void Initialize();
void Uninitialize();
void ClearScreen();
void RefreshScreen();
int GetPressedKey();
void GetSize(int *width, int *height);
void Sleep(int ms);

int DrawString(int x, int y, const std::string_view line,
               eColor color = eColor::WHITE);
void DrawCharacter(int x, int y, char character, eColor color = eColor::WHITE);
void DrawSymbol(int x, int y, eSymbol symbol);
void DrawTitleBar(int x, int y, int w, std::string text,
                  eColor color = eColor::WHITE);

}  // namespace terminal
}  // namespace exploratron
#endif