#pragma once

#include <cinttypes>
#include <map>

#include "motis/loader/hrd/model/hrd_service.h"

#include "motis/loader/hrd/builder/bitfield_builder.h"

#include "motis/schedule-format/Attribute_generated.h"

namespace motis {
namespace loader {
namespace hrd {

struct attribute_builder {
  attribute_builder(std::map<uint16_t, std::string> hrd_attributes);

  flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<Attribute>>>
  create_attributes(std::vector<hrd_service::attribute> const&,
                    bitfield_builder&, flatbuffers::FlatBufferBuilder&);

  flatbuffers::Offset<Attribute> get_or_create_attribute(
      hrd_service::attribute const&, bitfield_builder&,
      flatbuffers::FlatBufferBuilder&);

  std::map<uint16_t, std::string> const hrd_attributes_;
  std::map<uint16_t, flatbuffers::Offset<AttributeInfo>> fbs_attribute_infos_;
  std::map<std::pair<uint16_t, int>, flatbuffers::Offset<Attribute>>
      fbs_attributes_;
};

}  // namespace hrd
}  // namespace loader
}  // namespace motis
