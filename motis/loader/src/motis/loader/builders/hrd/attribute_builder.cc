#include "motis/loader/builders/hrd/attribute_builder.h"

#include "parser/util.h"

#include "motis/loader/util.h"

namespace motis {
namespace loader {
namespace hrd {

using namespace parser;
using namespace flatbuffers;

attribute_builder::attribute_builder(
    std::map<uint16_t, std::string> hrd_attributes)
    : hrd_attributes_(std::move(hrd_attributes)) {}

Offset<Vector<Offset<Attribute>>> attribute_builder::create_attributes(
    std::vector<hrd_service::attribute> const& attributes, bitfield_builder& bb,
    FlatBufferBuilder& fbb) {
  return fbb.CreateVector(
      transform_to_vec(begin(attributes), end(attributes),
                       [&](hrd_service::attribute const& attr) {
                         return get_or_create_attribute(attr, bb, fbb);
                       }));
}

Offset<Attribute> attribute_builder::get_or_create_attribute(
    hrd_service::attribute const& attr, bitfield_builder& bb,
    FlatBufferBuilder& fbb) {
  auto const attr_info_key = raw_to_int<uint16_t>(attr.code);
  auto const attr_key = std::make_pair(attr_info_key, attr.bitfield_num);

  return get_or_create(fbs_attributes_, attr_key, [&]() {
    auto const attr_info =
        get_or_create(fbs_attribute_infos_, attr_info_key, [&]() {
          auto const stamm_attributes_it = hrd_attributes_.find(attr_info_key);
          verify(stamm_attributes_it != end(hrd_attributes_),
                 "attribute with code %.*s not found\n",
                 (int)attr.code.length(), attr.code.c_str());

          auto const fbs_attribute_info = CreateAttributeInfo(
              fbb, to_fbs_string(fbb, attr.code),
              to_fbs_string(fbb, stamm_attributes_it->second, ENCODING));

          fbs_attribute_infos_[attr_info_key] = fbs_attribute_info;
          return fbs_attribute_info;
        });

    auto const fbs_attribute = CreateAttribute(
        fbb, attr_info, bb.get_or_create_bitfield(attr.bitfield_num, fbb));
    fbs_attributes_[attr_key] = fbs_attribute;
    return fbs_attribute;
  });
}

}  // hrd
}  // loader
}  // motis
