#pragma once

namespace motis {
namespace reliability {


class probability_distribution;


struct tt_distribution_manager {


  probability_distribution const& get_start_distribution(std::string const& train_category) const;

};

}  // namespace reliability
}  // namespace motis
