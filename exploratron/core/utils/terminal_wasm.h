#ifndef EXPLORATRON_CORE_UTILS_TERMINAL_WASM_H_
#define EXPLORATRON_CORE_UTILS_TERMINAL_WASM_H_

#include "exploratron/core/utils/terminal_interface.h"

namespace exploratron::terminal::wasm {

class Terminal : public TextGridTerminalInterface {
 public:
  void Initialize() override;
  void Uninitialize() override;
  void ClearScreen() override;
  void RefreshScreen() override;
  void GetSize(int *width, int *height) override;
  void Sleep(int ms) override;
  int GetPressedKey() override;
  void SetCell(eSymbol symbol, TextCell *cell) override;
};

}  // namespace exploratron::terminal::wasm

#endif