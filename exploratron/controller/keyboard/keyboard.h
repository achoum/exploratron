#ifndef EXPLORATRON_CONTROLLER_KEYBOARD_H_
#define EXPLORATRON_CONTROLLER_KEYBOARD_H_

#include <memory>

#include "exploratron/core/abstract_controller.h"

namespace exploratron {
namespace keyboard {
class KeyboardController : public AbstractController {
public:
  virtual ~KeyboardController() = default;
  KeyboardController();
  Output Step(const Input& input) override;
};

class KeyboardControllerBuilder : public AbstractControllerBuilder {
public:
  KeyboardControllerBuilder(const std::string_view path) {}
  virtual ~KeyboardControllerBuilder() = default;

  std::unique_ptr<AbstractController>
  Create(const MapDef &map_definition) const override {
    return std::make_unique<KeyboardController>();
  }

  std::string name() const override { return "KeyboardControllerBuilder"; }
};
} // namespace keyboard

REGISTER_AbstractControllerBuilder(keyboard::KeyboardControllerBuilder,
                                   KeyboardControllerBuilder, "Keyboard");

} // namespace exploratron

#endif