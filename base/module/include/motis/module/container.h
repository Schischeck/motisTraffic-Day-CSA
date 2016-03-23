#include <map>
#include <string>
#include <memory>

namespace motis {

template <typename T>
struct container_traits;

#define MOTIS_REGISTER_CONTAINER(T) \
  template <>                       \
  struct container_traits<T> {      \
    static const char* name;        \
  };                                \
  const char* container_traits<T>::name = #T

namespace module {

using container_ptr = std::shared_ptr<void>;
using snapshot = std::map<std::string, std::map<std::string, container_ptr>>;

template <typename T>
T* get_container(snapshot const& s, std::string const& version = "master") {
  auto ptr = s.at(container_traits<T>::name).at(version).get();
  return reinterpret_cast<T*>(ptr);
}

template <typename T>
void register_container(snapshot& s, T* container,
                        std::string const& version = "master") {
  s[container_traits<T>::name][version] = std::shared_ptr<void>(container);
}

}  // namespace module
}  // namespace motis
