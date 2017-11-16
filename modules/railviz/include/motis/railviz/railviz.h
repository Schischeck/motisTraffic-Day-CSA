#pragma once

#include "motis/module/module.h"

namespace motis {
namespace railviz {

struct train_retriever;

struct railviz : public motis::module::module {
  railviz();
  ~railviz() override;

  railviz(railviz const&) = delete;
  railviz& operator=(railviz const&) = delete;

  railviz(railviz&&) = delete;
  railviz& operator=(railviz&&) = delete;

  void init(motis::module::registry&) override;

private:
  motis::module::msg_ptr get_trip_guesses(motis::module::msg_ptr const&) const;
  motis::module::msg_ptr get_trains(motis::module::msg_ptr const&) const;
  motis::module::msg_ptr get_trips(motis::module::msg_ptr const&) const;
  motis::module::msg_ptr get_station(motis::module::msg_ptr const&) const;

  std::unique_ptr<train_retriever> train_retriever_;
};

}  // namespace railviz
}  // namespace motis
