#ifndef RAPIDJSON_WITH_EXCEPTION_H_
#define RAPIDJSON_WITH_EXCEPTION_H_

#include <stdexcept>

namespace rapidjson {

class rapidjson_error : public std::runtime_error {
 public:
  rapidjson_error(const char* what) : std::runtime_error(what) { }
  rapidjson_error() : std::runtime_error("json schema invalid") { }
};

}  // namespace rapidjson

#ifdef RAPIDJSON_ASSERT
#undef RAPIDJSON_ASSERT
#endif

#define RAPIDJSON_ASSERT(x) {\
  if(x) { \
  } else { throw rapidjson::rapidjson_error(#x); } \
}

#include <rapidjson/document.h>

#endif  // RAPIDJSON_WITH_EXCEPTION_H_
