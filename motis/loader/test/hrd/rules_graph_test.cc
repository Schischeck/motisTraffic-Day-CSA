#include <cinttypes>
#include <iostream>

#include "boost/filesystem.hpp"
#include "boost/range/iterator_range.hpp"

#include "gtest/gtest.h"

#include "test_spec.h"

#include "flatbuffers/flatbuffers.h"

#include "motis/core/common/logging.h"

#include "motis/schedule-format/ServiceRules_generated.h"
#include "motis/loader/util.h"
#include "motis/loader/parsers/hrd/bitfields_parser.h"
#include "motis/loader/parsers/hrd/bitfield_translator.h"
#include "motis/loader/parsers/hrd/service/split_service.h"
#include "motis/loader/parsers/hrd/service_rules/service_rules.h"
#include "motis/loader/parsers/hrd/service_rules/through_services_parser.h"
#include "motis/loader/parsers/hrd/service_rules/merge_split_rules_parser.h"

namespace motis {
namespace loader {
namespace hrd {

using namespace boost::filesystem;
using namespace motis::logging;

class rule_services_test : public ::testing::Test {
protected:
  rule_services_test(std::string schedule_name)
      : schedule_name_(std::move(schedule_name)) {}

  virtual void SetUp() {
    path const root = SCHEDULES / schedule_name_;
    LOG(info) << "using schedule: " << root.c_str();

    path const stamm = root / "stamm";
    filenames_.emplace_back("bitfield.101");
    specs_.emplace_back(stamm, filenames_.back().c_str());
    auto hrd_bitfields = parse_bitfields(specs_.back().lf_);

    path const services_root = root / "fahrten";
    std::vector<std::string> services_filenames;
    for (auto const& entry :
         boost::make_iterator_range(directory_iterator(services_root), {})) {
      if (is_regular(entry.path())) {
        services_filenames.emplace_back(entry.path().filename().string());
      }
    }

    flatbuffers::FlatBufferBuilder fbb;
    bitfield_translator bt(hrd_bitfields, fbb);
    std::vector<hrd_service> expanded_services;
    for (auto const& services_filename : services_filenames) {
      filenames_.emplace_back(services_filename);
      specs_.emplace_back(services_root, filenames_.back().c_str());

      LOG(info) << "load hrd services file: " << specs_.back().lf_.name;
      for (auto const& s : specs_.back().get_hrd_services()) {
        expand_traffic_days(s, bt, expanded_services);
      }
    }

    rules rs;
    filenames_.emplace_back("durchbi.101");
    specs_.emplace_back(stamm, filenames_.back().c_str());
    parse_through_service_rules(specs_.back().lf_, hrd_bitfields, rs);
    filenames_.emplace_back("vereinig_vt.101");
    specs_.emplace_back(stamm, filenames_.back().c_str());
    parse_merge_split_service_rules(specs_.back().lf_, hrd_bitfields, rs);

    service_rules_ = service_rules(rs);
    for (auto const& s : expanded_services) {
      service_rules_.add_service(s);
    }
    service_rules_.create_graph();
  }

  std::string schedule_name_;
  service_rules service_rules_;

private:
  std::vector<test_spec> specs_;
  std::vector<std::string> filenames_;
};

class ts_length_1 : public rule_services_test {
public:
  ts_length_1() : rule_services_test("ts-length-1") {}
};

class ts_length_2 : public rule_services_test {
public:
  ts_length_2() : rule_services_test("ts-length-2") {}
};

TEST_F(ts_length_1, rule_services) {
  // check remaining services
  ASSERT_EQ(1, service_rules_.origin_services_.size());

  auto const& remaining_service = service_rules_.origin_services_[0].get();
  ASSERT_EQ(bitfield{"0001110"}, remaining_service->traffic_days_);

  // check rule services
  ASSERT_EQ(1, service_rules_.rule_services_.size());

  auto const& rule_service = service_rules_.rule_services_[0];
  std::for_each(begin(rule_service.rules), end(rule_service.rules),
                [](service_rule const& sr) {
                  ASSERT_EQ(bitfield{"0010001"}, sr.s1->traffic_days_);
                  ASSERT_EQ(bitfield{"0010001"}, sr.s2->traffic_days_);
                });
}

TEST_F(ts_length_2, rule_services) {
  // check remaining services
  ASSERT_EQ(0, service_rules_.origin_services_.size());

  // check rule services
  ASSERT_EQ(2, service_rules_.rule_services_.size());

  auto const& rule_service1 = service_rules_.rule_services_[0];
  std::for_each(begin(rule_service1.rules), end(rule_service1.rules),
                [](service_rule const& sr) {
                  ASSERT_EQ(bitfield{"0011111"}, sr.s1->traffic_days_);
                  ASSERT_EQ(bitfield{"0011111"}, sr.s2->traffic_days_);
                });

  auto const& rule_service2 = service_rules_.rule_services_[1];
  std::for_each(begin(rule_service2.rules), end(rule_service2.rules),
                [](service_rule const& sr) {
                  ASSERT_EQ(bitfield{"1100000"}, sr.s1->traffic_days_);
                  ASSERT_EQ(bitfield{"1100000"}, sr.s2->traffic_days_);
                });
}

}  // loader
}  // motis
}  // hrd
