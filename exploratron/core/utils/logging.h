#ifndef EXPLORATRON_CORE_UTILS_LOGGING_H_
#define EXPLORATRON_CORE_UTILS_LOGGING_H_

#include <iostream>
#include <string_view>

enum Severity { INFO, FATAL };

// Logging.
//
// Usage example:
//   LOG(INFO) << "Hello world";
//
#define LOG(sev) LOG_##sev

#define LOG_INFO ::internal::LogMessage(INFO, __FILE__, __LINE__)
#define LOG_FATAL ::internal::FatalLogMessage(__FILE__, __LINE__)
// Evaluates an expression returning a boolean. If the returned value is false
// emit a fatal error message.
//
// Usage example:
//   CHECK(x==y) << "Hello world";
//
#ifndef CHECK
// NOLINTBEGIN
#define CHECK(expr)                                                            \
  if (!(expr))                                                                 \
  LOG(FATAL) << "Check failed " #expr
// NOLINTEND
#endif

// Like check in debug mode. No-op in release mode.
//
// Usage example:
//   DCHECK(x==y) << "Hello world";
//
#ifndef NDEBUG
// NOLINTBEGIN
#define DCHECK(expr)                                                           \
  if (!(expr))                                                                 \
  LOG(FATAL) << "Check failed " #expr
// NOLINTEND
#else
#define DCHECK(expr) ::internal::NullSink()
#endif

// Check helpers.
//
// Usage example:
//   CHECK_EQ(x, y) << "Hello world";
//
#ifndef CHECK_EQ
#define CHECK_EQ(a, b) CHECK(a == b) << " with a=" << a << " and b=" << b
#define CHECK_NE(a, b) CHECK(a != b) << " with a=" << a << " and b=" << b
#define CHECK_GE(a, b) CHECK(a >= b) << " with a=" << a << " and b=" << b
#define CHECK_LE(a, b) CHECK(a <= b) << " with a=" << a << " and b=" << b
#define CHECK_GT(a, b) CHECK(a > b) << " with a=" << a << " and b=" << b
#define CHECK_LT(a, b) CHECK(a < b) << " with a=" << a << " and b=" << b

#define DCHECK_EQ(a, b) DCHECK(a == b) << " with a=" << a << " and b=" << b
#define DCHECK_NE(a, b) DCHECK(a != b) << " with a=" << a << " and b=" << b
#define DCHECK_GE(a, b) DCHECK(a >= b) << " with a=" << a << " and b=" << b
#define DCHECK_LE(a, b) DCHECK(a <= b) << " with a=" << a << " and b=" << b
#define DCHECK_GT(a, b) DCHECK(a > b) << " with a=" << a << " and b=" << b
#define DCHECK_LT(a, b) DCHECK(a < b) << " with a=" << a << " and b=" << b
#endif

namespace internal {

// Extraction the filename from a path.
inline std::string_view ExtractFilename(std::string_view path) {
  auto last_sep = path.find_last_of("/\\");
  if (last_sep == std::string::npos) {
    // Start of filename no found.
    return path;
  }
  return path.substr(last_sep + 1);
}

class LogMessage {
public:
  LogMessage(Severity sev, std::string_view file, int line) : sev_(sev) {
    std::clog << "[";
    switch (sev) {
    case INFO:
      std::clog << "INFO";
      break;
    case FATAL:
      std::clog << "FATAL";
      break;
    default:
      std::clog << "UNDEF";
      break;
    }
    std::clog << " " << ExtractFilename(file) << ":" << line << "] ";
  }

  virtual ~LogMessage() { std::clog << std::endl; }

  template <typename T> LogMessage &operator<<(const T &v) {
    std::clog << v;
    return *this;
  }

protected:
  Severity sev_;
};

class FatalLogMessage : public LogMessage {
public:
  FatalLogMessage(std::string_view file, int line)
      : LogMessage(FATAL, file, line) {}

  [[noreturn]] ~FatalLogMessage() {
    std::clog << std::endl;
    std::clog.flush();
    std::abort();
  }
};

// Consumes and ignores all input streams.
class NullSink {
public:
  template <typename T> NullSink &operator<<(const T &v) { return *this; }
};

} // namespace internal

#endif
