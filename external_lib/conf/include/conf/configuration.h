#ifndef CONF_CONFIGURATION_H_
#define CONF_CONFIGURATION_H_

#include <ostream>

namespace boost {
namespace program_options {
class options_description;
class variables_map;
}
}

namespace conf {

class configuration {
public:
  virtual ~configuration() { }
  virtual boost::program_options::options_description desc() = 0;
  virtual void print(std::ostream& out) const = 0;

  friend std::ostream& operator<<(std::ostream& out,
                                  configuration const& conf) {
    conf.print(out);
    return out;
  }
};

}  // namespace conf

#endif  // CONF_CONFIGURATION_H_