#pragma once

#include "motis/module/module.h"

namespace motis {
namespace railviz {

struct train_retriever;

struct railviz : public motis::module::module {
  railviz();
  ~railviz();

  void init(motis::module::registry&) override;

private:
  motis::module::msg_ptr get_trains(motis::module::msg_ptr const&) const;

  std::unique_ptr<train_retriever> train_retriever_;
};

}  // namespace railviz
}  // namespace motis
