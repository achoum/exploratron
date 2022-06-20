#ifndef EXPLORATRON_CORE_UTILS_REGISTER_H_
#define EXPLORATRON_CORE_UTILS_REGISTER_H_

#include <mutex>
#include <string>
#include <vector>

#include "exploratron/core/utils/logging.h"

// Build the registration pool name from the interface class name.
#define INTERNAL_REGISTERER_CLASSNAME(INTERFACE) INTERFACE##Registerer

// Register a new pool of classes.
//
// Args:
//   cls: Interface class.
//   ...: Argument of the implementation class constructors.
//
#define REGISTRATION_CREATE_POOL(INTERFACE, ...)                               \
  class INTERNAL_REGISTERER_CLASSNAME(INTERFACE)                               \
      : public ::exploratron::ClassPool<INTERFACE, ##__VA_ARGS__> {  \
  public:                                                                      \
    template <typename IMPLEMENTATION>                                         \
    static ::exploratron::Empty                                      \
    Register(const std::string_view key) {                                     \
      if (IsName(key))                                                         \
        return {};                                                             \
      InternalGetItems()->push_back(                                           \
          std::make_unique<::exploratron::Creator<                   \
              INTERFACE, IMPLEMENTATION, ##__VA_ARGS__>>(key));                \
      return {};                                                               \
    }                                                                          \
  };

// Adds an implementation class to an existing pool.
#define REGISTRATION_REGISTER_CLASS(IMPLEMENTATION, IMPLEMENTATIONNAME, name,  \
                                    INTERFACE)                                 \
  static const auto register_##IMPLEMENTATIONNAME##_in_##INTERFACE =           \
      INTERNAL_REGISTERER_CLASSNAME(INTERFACE)::Register<IMPLEMENTATION>(      \
          name);

namespace exploratron {

struct Empty {};

template <class Interface, class... Args> class AbstractCreator {
public:
  virtual ~AbstractCreator() = default;
  AbstractCreator(std::string_view name) : name_(name) {}
  const std::string &name() const { return name_; }
  virtual std::unique_ptr<Interface> Create(Args... args) = 0;

private:
  std::string name_;
};

template <class Interface, class Implementation, class... Args>
class Creator final : public AbstractCreator<Interface, Args...> {
public:
  Creator(std::string_view name) : AbstractCreator<Interface, Args...>(name) {}
  std::unique_ptr<Interface> Create(Args... args) override {
    return std::make_unique<Implementation>(args...);
  };
};

template <class Interface, class... Args> class ClassPool {
public:
  static std::vector<std::unique_ptr<AbstractCreator<Interface, Args...>>> *
  InternalGetItems() {
    static std::vector<std::unique_ptr<AbstractCreator<Interface, Args...>>>
        items;
    return &items;
  }

  static std::vector<std::string> GetNames() { return InternalGetNames(); }

  static bool IsName(std::string_view name) {
    auto &items = *InternalGetItems();
    for (const auto &item : items) {
      if (name == item->name()) {
        return true;
      }
    }
    return false;
  }

  static std::unique_ptr<Interface> Create(std::string_view name,
                                           Args... args) {
    auto &items = *InternalGetItems();
    for (const auto &item : items) {
      if (name != item->name()) {
        continue;
      }
      return item->Create(args...);
    }
    std::string existing;
    for (const auto &item : items) {
      existing += " " + item->name();
    }
    LOG(FATAL) << "Unknown item \"" << name << "\" in class pool "
               << typeid(Interface).name() << ". Registered elements are \""
               << existing << "\"";
  }

private:
  static std::vector<std::string> InternalGetNames() {
    std::vector<std::string> names;
    auto &items = *InternalGetItems();
    for (const auto &item : items) {
      names.push_back(item->name());
    }
    return names;
  }
};

} // namespace exploratron

#endif // EXPLORATRON_CORE_UTILS_REGISTER_H_
