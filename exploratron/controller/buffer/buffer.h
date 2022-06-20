#ifndef EXPLORATRON_CONTROLLER_GENETIC_H_
#define EXPLORATRON_CONTROLLER_GENETIC_H_

#include <memory>
#include <queue>
#include <random>
#include <string>
#include <unordered_map>

#include "exploratron/core/abstract_controller.h"

namespace exploratron {
namespace buffer {

class BufferInput {
  public:
  void Add(Output output){
    outputs_.push(output);
  }

  std::queue<Output> outputs_;
};

class BufferController : public AbstractController {
 public:
  virtual ~BufferController() = default;
  BufferController(BufferInput* buffer_input) : buffer_input_(buffer_input) {}
  Output Step(const Input& input) override;

 private:
  BufferInput* buffer_input_;
};

class BufferControllerBuilder : public AbstractControllerBuilder {
 public:
  BufferControllerBuilder(const std::string_view path) {
    DCHECK(false);  // Not supported.
  }
  BufferControllerBuilder(BufferInput* buffer_input)
      : buffer_input_(buffer_input) {}
  virtual ~BufferControllerBuilder() = default;

  std::unique_ptr<AbstractController> Create(
      const MapDef& map_definition) const override {
    return std::make_unique<BufferController>(buffer_input_);
  }

  std::string name() const override { return "BufferControllerBuilder"; }

 private:
  BufferInput* buffer_input_;
};
}  // namespace buffer

// Not supported
// REGISTER_AbstractControllerBuilder(buffer::BufferControllerBuilder,
//                                   BufferControllerBuilder, "Buffer");

}  // namespace exploratron

#endif