#include "exploratron/controller/keyboard/keyboard.h"

#include "exploratron/core/utils/logging.h"
#include "exploratron/core/utils/terminal.h"

namespace exploratron {
namespace keyboard {
KeyboardController::KeyboardController() {}

Output KeyboardController::Step(const Input &input) {
  Output output;
  const auto value = terminal::GetPressedKey();
  switch (value) {
  case 'z':
  case terminal::kKeyArrowUp:
    output.move = eDirection::UP;
    break;
  case terminal::kKeyArrowLeft:
  case 'q':
    output.move = eDirection::LEFT;
    break;
  case 's':
  case terminal::kKeyArrowDown:
    output.move = eDirection::DOWN;
    break;
  case 'd':
  case terminal::kKeyArrowRight:
    output.move = eDirection::RIGHT;
    break;
  case terminal::kKeyEscape:
  case 'Q':
    output.stop = true;
  }
  return output;
}

} // namespace keyboard
} // namespace exploratron