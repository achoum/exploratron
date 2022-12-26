#include "exploratron/core/utils/symbols.h"

#include "exploratron/core/utils/logging.h"

namespace exploratron::terminal {

// Color code in linux terminals.
int LinuxColor(const eColor color) {
  switch (color) {
    case eColor::WHITE:
      return 37;
    case eColor::RED:
      return 31;
    case eColor::BLUE:
      return 34;
    case eColor::GREEN:
      return 32;
    case eColor::GRAY:
      return 90;
    case eColor::VIOLET:
      return 35;
    case eColor::YELLOW:
      return 33;
  }
  LOG(FATAL) << "Color not specified";
}

}  // namespace exploratron::terminal