#include "motis/address/address.h"

#include "boost/filesystem.hpp"

#include "utl/to_vec.h"

#include "address-typeahead/typeahead.h"

#include "motis/core/common/logging.h"

namespace fs = boost::filesystem;
using namespace motis::module;
using namespace address_typeahead;

namespace motis {
namespace address {

struct address::impl {
  explicit impl(std::string const& path) : typeahead_(path) {}

  msg_ptr get_guesses(msg_ptr const& msg) {
    message_creator fbb;
    fbb.create_and_finish(
        MsgContent_AddressResponse,
        CreateAddressResponse(
            fbb, fbb.CreateVector(utl::to_vec(
                     typeahead_.complete(
                         motis_content(AddressRequest, msg)->input()->str()),
                     [&](typeahead::search_result const& r) {
                       auto const pos = Position{r.lat, r.lon};
                       auto sorted_regions = r.regions;
                       std::sort(begin(sorted_regions), end(sorted_regions),
                                 [](region const& a, region const& b) {
                                   return a.admin_level > b.admin_level;
                                 });
                       return CreateAddress(
                           fbb, &pos, fbb.CreateString(r.name),
                           fbb.CreateString(r.type),
                           fbb.CreateVector(utl::to_vec(
                               sorted_regions, [&](region const& region) {
                                 return CreateRegion(
                                     fbb, fbb.CreateString(region.name),
                                     region.admin_level);
                               })));
                     })))
            .Union());
    return make_msg(fbb);
  }

  typeahead typeahead_;
};

address::address() : module("Address Typeahead", "address") {
  string_param(db_path_, "address_db", "db", "address typeahead database path");
}

address::~address() = default;

void address::init(motis::module::registry& reg) {
  if (!fs::is_directory(db_path_)) {
    LOG(logging::warn) << db_path_ << " is not a not a directory, skipping";
    return;
  }

  try {
    impl_ = std::make_unique<impl>(db_path_);
    reg.register_op("/address", std::bind(&impl::get_guesses, impl_.get(),
                                          std::placeholders::_1));
  } catch (std::exception const& e) {
    LOG(logging::warn) << "address module not initialized (" << e.what() << ")";
  }
}

}  // namespace address
}  // namespace motis
