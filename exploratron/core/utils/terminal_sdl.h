#ifndef EXPLORATRON_CORE_UTILS_TERMINAL_SDL_H_
#define EXPLORATRON_CORE_UTILS_TERMINAL_SDL_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

#include "exploratron/core/utils/terminal_interface.h"

namespace exploratron::terminal::sdl {

class Terminal : public TerminalInterface {
 public:
  void Initialize() override;
  void Uninitialize() override;
  void ClearScreen() override;
  void RefreshScreen() override;
  void GetSize(int *width, int *height) override;
  int GetPressedKey() override;
  void DrawCharacter(int x, int y, char character, eColor color) override;
  void DrawCharacterPixel(int x, int y, char character, eColor color);
  int DrawString(int x, int y, const std::string_view line,
                 eColor color) override;
  void DrawSymbol(int x, int y, eSymbol symbol) override;
  void Sleep(int ms) override;
  void DrawTitleBar(int x, int y, int w, std::string text,
                    eColor color) override;

 private:
  SDL_Texture *LoadTexture(std::string_view path);
  int NumCols(SDL_Texture *texture, int col_width);
  void CreateFontBitmap();

  // Cell size on screen.
  static constexpr int kCellWidth = 24;
  static constexpr int kCellHeight = 24;

  // Symbol size in PNG file.
  static constexpr int kSymbolWidth = 24;
  static constexpr int kSymbolHeight = 24;

  // Font size.
  static constexpr int kFontWidth = 24;
  static constexpr int kFontHeight = 24;
  static constexpr int kFontPtSize = 24;

  SDL_Window *window_ = nullptr;
  SDL_Renderer *renderer_ = nullptr;
  SDL_Texture *symbol_ = nullptr;

  SDL_Texture *font_texture_ = nullptr;
  TTF_Font *font_ = NULL;
  int num_font_texture_cols = -1;

  int num_symbol_cols = -1;
};

}  // namespace exploratron::terminal::sdl

#endif