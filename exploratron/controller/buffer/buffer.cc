#include "exploratron/controller/buffer/buffer.h"
#include "exploratron/core/utils/logging.h"

namespace exploratron {
namespace buffer {

Output BufferController::Step(const Input& input) {
  DCHECK(!buffer_input_->outputs_.empty());
  auto ret = std::move(buffer_input_->outputs_.front());
  buffer_input_->outputs_.pop();
  return ret;
}

}  // namespace buffer
}  // namespace exploratron