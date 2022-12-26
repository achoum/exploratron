#include "exploratron/core/utils/terminal_sdl_wasm.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

namespace exploratron::terminal::sdl_wasm {

uint8_t kColorEnumToRGB[(int)eColor::_NUM_COLOR][3] = {
    {255, 255, 255},  // WHITE
    {255, 0, 0},      // RED,
    {0, 255, 0},      // BLUE,
    {0, 0, 255},      // GREEN,
    {100, 100, 100},  // GRAY,
    {255, 0, 255},    // VIOLET,
    {255, 255, 0},    // YELLOW,
};

void Terminal::Initialize() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    LOG(FATAL) << "Cannot initialize SDL. Error: " << SDL_GetError();
  }

  window_ = SDL_CreateWindow(
      "Exploratron", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1920,
      1080, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_INPUT_FOCUS);
  if (!window_) {
    LOG(FATAL) << "Cannot create SDL window. Error: " << SDL_GetError();
  }

  renderer_ = SDL_CreateRenderer(
      window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!renderer_) {
    LOG(FATAL) << "Cannot create rendered";
  }

  SDL_SetRenderDrawColor(renderer_, 0x00, 0x00, 0x00, 0x00);

  int imgFlags = IMG_INIT_PNG;
  if (!(IMG_Init(imgFlags) & imgFlags)) {
    LOG(FATAL) << "Cannot initialize PNG loading. Error: " << SDL_GetError();
  }

  if (TTF_Init() == -1) {
    LOG(FATAL) << "Cannot initialize TFF loading. Error: " << SDL_GetError();
  }

  symbol_ = LoadTexture("exploratron/assets/graphic/symbols.png");
  num_symbol_cols = NumCols(symbol_, kSymbolWidth);

  CreateFontBitmap();
}

void Terminal::CreateFontBitmap() {
  font_ = TTF_OpenFont("exploratron/assets/graphic/consola.ttf", kFontPtSize);
  if (!font_) {
    LOG(FATAL) << "Cannot load font";
  }

  const int texture_width = 512;
  const int texture_height = 512;

  SDL_Surface *font_surface = SDL_CreateRGBSurfaceWithFormat(
      0, texture_width, texture_height, 32,
      SDL_PixelFormatEnum::SDL_PIXELFORMAT_RGBA32);

  CHECK(font_surface);
  num_font_texture_cols = texture_width / kFontWidth;

  SDL_Color textColor = {255, 255, 255};
  for (int i = 0; i < 128; i++) {
    char text[2] = {i, 0};
    SDL_Surface *character_surface =
        TTF_RenderText_Blended(font_, text, textColor);
    if (!character_surface) {
      LOG(INFO) << "Cannot print characer " << i
                << " Error: " << SDL_GetError();
      continue;
    }

    SDL_Rect dst_rect{
        .x = (i % num_font_texture_cols) * kFontWidth,
        .y = (i / num_font_texture_cols) * kFontHeight,
        .w = character_surface->w,
        .h = character_surface->h,
    };

    SDL_UpperBlit(character_surface, &character_surface->clip_rect,
                  font_surface, &dst_rect);
    SDL_FreeSurface(character_surface);
  }

  font_texture_ = SDL_CreateTextureFromSurface(renderer_, font_surface);
  CHECK(font_texture_);
  SDL_FreeSurface(font_surface);
}

void Terminal::Uninitialize() {
  SDL_DestroyTexture(symbol_);

  TTF_CloseFont(font_);

  SDL_DestroyRenderer(renderer_);
  SDL_DestroyWindow(window_);
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
}

void Terminal::GetSize(int *width, int *height) {
  *width = 800;
  *height = 600;
}

void Terminal::ClearScreen() { SDL_RenderClear(renderer_); }

void Terminal::RefreshScreen() { SDL_RenderPresent(renderer_); }

void Terminal::DrawCharacter(int x, int y, char character, eColor color) {
  DrawCharacterPixel(x * kCellWidth, y * kCellHeight, character, color);
}

void Terminal::DrawCharacterPixel(int x, int y, char character, eColor color) {
  int symbol_idx = (int)character;
  SDL_Rect src_rect{
      .x = (symbol_idx % num_font_texture_cols) * kFontWidth,
      .y = (symbol_idx / num_font_texture_cols) * kFontHeight,
      .w = kFontWidth,
      .h = kFontHeight,
  };
  SDL_Rect dst_rect{
      .x = x,
      .y = y,
      .w = kFontWidth,
      .h = kFontHeight,
  };

  SDL_SetTextureColorMod(font_texture_, kColorEnumToRGB[(int)color][0],
                         kColorEnumToRGB[(int)color][1],
                         kColorEnumToRGB[(int)color][2]);
  SDL_RenderCopy(renderer_, font_texture_, &src_rect, &dst_rect);
}

int Terminal::DrawString(int x, int y, const std::string_view line,
                         eColor color) {
  // Operate in PX mode.
  x *= kCellWidth;
  y *= kCellHeight;

  auto save_x = x;
  for (auto character : line) {
    if (character == '\n') {
      y += kCellHeight;
      x = save_x;
      continue;
    }
    DrawCharacterPixel(x, y, character, color);
    x += kCellWidth / 2;
  }
  return line.size();
}

void Terminal::DrawSymbol(int x, int y, eSymbol symbol) {
  int symbol_idx = (int)symbol;
  SDL_Rect src_rect{
      .x = (symbol_idx % num_symbol_cols) * kSymbolWidth,
      .y = (symbol_idx / num_symbol_cols) * kSymbolHeight,
      .w = kSymbolWidth,
      .h = kSymbolHeight,
  };
  SDL_Rect dst_rect{
      .x = x * kCellWidth,
      .y = y * kCellHeight,
      .w = kCellWidth,
      .h = kCellHeight,
  };
  SDL_RenderCopy(renderer_, symbol_, &src_rect, &dst_rect);
}

int Terminal::GetPressedKey() {
  SDL_Event e;

  while (true) {
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        exit(0);
      }

      if (e.type != SDL_KEYDOWN) {
        continue;
      }

      switch (e.key.keysym.sym) {
        case SDLK_RETURN:
          return kKeyReturn;
        case SDLK_ESCAPE:
          return kKeyEscape;
        case SDLK_DOWN:
          return kKeyArrowDown;
        case SDLK_UP:
          return kKeyArrowUp;
        case SDLK_LEFT:
          return kKeyArrowLeft;
        case SDLK_RIGHT:
          return kKeyArrowRight;
      }

      if (e.key.keysym.sym < 128) {
        return e.key.keysym.sym;
      }
    }

    Sleep(20);
    return 0;
  }
}

void Terminal::Sleep(int ms) { SDL_Delay(ms); }

SDL_Texture *Terminal::LoadTexture(std::string_view path) {
  SDL_Surface *surface = IMG_Load(std::string(path).c_str());
  if (!surface) {
    LOG(FATAL) << "Unable to create texture at " << path
               << ". Error:" << SDL_GetError();
  }
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer_, surface);
  if (!texture) {
    LOG(FATAL) << "Unable to create texture at " << path
               << ". Error:" << SDL_GetError();
  }
  SDL_FreeSurface(surface);
  return texture;
}

int Terminal::NumCols(SDL_Texture *texture, int col_width) {
  Uint32 format;
  int access, w, h;
  if (SDL_QueryTexture(texture, &format, &access, &w, &h) < 0) {
    LOG(FATAL) << "Cannot query texture";
  }
  return w / col_width;
}

}  // namespace exploratron::terminal::sdl