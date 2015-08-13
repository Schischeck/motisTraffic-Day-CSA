#include <string>

#include "motis/converter/Converter_generated.h"

namespace motis {
namespace loader {

inline std::string fbs_str_offset_to_str(
    flatbuffers::Offset<flatbuffers::String> fbs_str,
    flatbuffers::FlatBufferBuilder& b) {
  b.Finish(CreateConverter(b, fbs_str));
  return GetConverter(b.GetBufferPointer())->str()->str();
}

}  // loader
}  // motis
