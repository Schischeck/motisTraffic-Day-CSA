#pragma once

#include <string>

#include "boost/lexical_cast.hpp"

#include "motis/core/schedule/schedule.h"
#include "motis/core/access/error.h"

namespace motis {

inline int output_train_nr(uint32_t const& train_nr,
                           uint32_t original_train_nr) {
  return train_nr <= kMaxValidTrainNr ? train_nr : original_train_nr;
}

inline std::string get_service_name(schedule const& sched,
                                    connection_info const* info) {
  auto const& cat_name = sched.categories[info->family]->name;
  auto const& line_identifier = info->line_identifier;

  std::string print_train_nr;
  auto const& train_nr =
      output_train_nr(info->train_nr, info->original_train_nr);
  if (train_nr != 0) {
    print_train_nr = boost::lexical_cast<std::string>(train_nr);
  } else if (train_nr == 0 && !line_identifier.empty()) {
    print_train_nr = line_identifier;
  } else {
    print_train_nr = "";
  }

  std::string name;
  switch (sched.categories[info->family]->output_rule) {
    case category::CATEGORY_AND_TRAIN_NUM:
      name = cat_name + " " + print_train_nr;
      break;

    case category::CATEGORY: name = cat_name; break;

    case category::TRAIN_NUM: name = print_train_nr; break;

    case category::NOTHING: break;

    case category::PROVIDER_AND_TRAIN_NUM:
      if (info->provider_ != nullptr) {
        name = info->provider_->short_name + " ";
      }
      name += print_train_nr;
      break;

    case category::PROVIDER:
      if (info->provider_ != nullptr) {
        name = info->provider_->short_name;
      }
      break;

    case category::LINE:
      if (!line_identifier.empty()) {
        name = line_identifier;
        break;
      }
    // fall-through

    case category::CATEGORY_AND_LINE:
      name = cat_name + " " + line_identifier;
      break;
  }

  return name;
}

}  // namespace motis
