#pragma once

#include <memory>
#include <vector>

#include "motis/module/module.h"

namespace motis {
namespace rt {

struct rt_handler;

struct rt : public motis::module::module {
  rt();
  ~rt() override;

  rt(rt const&) = delete;
  rt& operator=(rt const&) = delete;

  rt(rt&&) = delete;
  rt& operator=(rt&&) = delete;

  std::string name() const override { return "rt"; }

  boost::program_options::options_description desc() override;
  void print(std::ostream&) const override {}
  bool empty_config() const override { return true; }

  void init(motis::module::registry&) override;

private:
  std::unique_ptr<rt_handler> handler_;
};

}  // namespace rt
}  // namespace motis
