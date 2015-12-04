#include <cinttypes>
#include <iostream>

#include "boost/filesystem.hpp"
#include "boost/range/iterator_range.hpp"

#include "motis/loader/hrd/builder/bitfield_builder.h"
#include "gtest/gtest.h"

#include "flatbuffers/flatbuffers.h"

#include "motis/core/common/logging.h"

#include "motis/schedule-format/RuleService_generated.h"
#include "motis/loader/util.h"
#include "motis/loader/hrd/model/split_service.h"
#include "motis/loader/hrd/parser/bitfields_parser.h"
#include "motis/loader/hrd/parser/through_services_parser.h"
#include "motis/loader/hrd/parser/merge_split_rules_parser.h"
#include "motis/loader/hrd/parser/service_parser.h"
#include "motis/loader/hrd/builder/rule_service_builder.h"

#include "./test_spec_test.h"

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
    path const stamm = root / "stamm";
    path const fahrten = root / "fahrten";

    // load bitfields
    flatbuffers::FlatBufferBuilder fbb;
    data_.push_back(loaded_file(stamm / "bitfield.101"));
    bitfield_builder bb(parse_bitfields(data_.back()));

    // load service rules
    service_rules rs;
    data_.push_back(loaded_file(stamm / "durchbi.101"));
    parse_through_service_rules(data_.back(), bb.hrd_bitfields_, rs);
    data_.push_back(loaded_file(stamm / "vereinig_vt.101"));
    parse_merge_split_service_rules(data_.back(), bb.hrd_bitfields_, rs);

    // load services and create rule services
    rsb_ = rule_service_builder(rs);
    std::vector<path> services_files;
    collect_files(fahrten, services_files);
    for (auto const& services_file : services_files) {
      data_.push_back(loaded_file(services_file));
      for_each_service(data_.back(), bb.hrd_bitfields_,
                       [&](hrd_service const& s) { rsb_.add_service(s); });
    }
    rsb_.resolve_rule_services();

    // remove all remaining services that does not have any traffic day left
    rsb_.origin_services_.erase(
        std::remove_if(begin(rsb_.origin_services_), end(rsb_.origin_services_),
                       [](std::unique_ptr<hrd_service> const& service_ptr) {
                         return service_ptr.get()->traffic_days_.none();
                       }),
        end(rsb_.origin_services_));
  }

  std::string schedule_name_;
  rule_service_builder rsb_;

private:
  std::vector<loaded_file> data_;
};

class loader_ts_once : public rule_services_test {
public:
  loader_ts_once() : rule_services_test("ts-once") {}
};

class loader_ts_twice : public rule_services_test {
public:
  loader_ts_twice() : rule_services_test("ts-twice") {}
};

class loader_ts_twice_all_combinations : public rule_services_test {
public:
  loader_ts_twice_all_combinations()
      : rule_services_test("ts-twice-all-combinations") {}
};

class loader_ts_2_to_1 : public rule_services_test {
public:
  loader_ts_2_to_1() : rule_services_test("ts-2-to-1") {}
};

class loader_ts_2_to_1_cycle : public rule_services_test {
public:
  loader_ts_2_to_1_cycle() : rule_services_test("ts-2-to-1-cycle") {}
};

class loader_ts_twice_2_to_1_cycle : public rule_services_test {
public:
  loader_ts_twice_2_to_1_cycle()
      : rule_services_test("ts-twice-2-to-1-cycle") {}
};

class loader_ts_passing_service : public rule_services_test {
public:
  loader_ts_passing_service() : rule_services_test("ts-passing-service") {}
};

class loader_mss_once : public rule_services_test {
public:
  loader_mss_once() : rule_services_test("mss-once") {}
};

class loader_mss_twice : public rule_services_test {
public:
  loader_mss_twice() : rule_services_test("mss-twice") {}
};

class loader_mss_many : public rule_services_test {
public:
  loader_mss_many() : rule_services_test("mss-many") {}
};

TEST_F(loader_ts_once, rule_services) {
  // check remaining services
  ASSERT_EQ(1, rsb_.origin_services_.size());

  auto const& remaining_service = rsb_.origin_services_[0].get();
  ASSERT_EQ(bitfield{"0001110"}, remaining_service->traffic_days_);

  // check rule services
  ASSERT_EQ(1, rsb_.rule_services_.size());

  auto const& rule_service = rsb_.rule_services_[0];
  for (auto const& sr : rule_service.rules) {
    ASSERT_EQ(RuleType_THROUGH, sr.rule_info.type);
    ASSERT_EQ(bitfield{"0010001"}, sr.s1->traffic_days_);
    ASSERT_EQ(bitfield{"0010001"}, sr.s2->traffic_days_);
  }
}

TEST_F(loader_ts_twice, rule_services) {
  // check remaining services
  ASSERT_EQ(0, rsb_.origin_services_.size());

  // check rule services
  ASSERT_EQ(2, rsb_.rule_services_.size());

  auto const& rule_service1 = rsb_.rule_services_[0];
  ASSERT_EQ(3, rule_service1.services.size());
  ASSERT_EQ(2, rule_service1.rules.size());
  for (auto const& sr : rule_service1.rules) {
    ASSERT_EQ(RuleType_THROUGH, sr.rule_info.type);
    ASSERT_EQ(bitfield{"0011111"}, sr.s1->traffic_days_);
    ASSERT_EQ(bitfield{"0011111"}, sr.s2->traffic_days_);
  }
  auto const& rule_service2 = rsb_.rule_services_[1];
  ASSERT_EQ(2, rule_service2.services.size());
  ASSERT_EQ(1, rule_service2.rules.size());
  for (auto const& sr : rule_service2.rules) {
    ASSERT_EQ(RuleType_THROUGH, sr.rule_info.type);
    ASSERT_EQ(bitfield{"1100000"}, sr.s1->traffic_days_);
    ASSERT_EQ(bitfield{"1100000"}, sr.s2->traffic_days_);
  }
}

TEST_F(loader_ts_twice_all_combinations, rule_services) {
  // check remaining services
  ASSERT_EQ(3, rsb_.origin_services_.size());
  auto const& service1 = rsb_.origin_services_[0];
  ASSERT_EQ(bitfield{"0000001"}, service1->traffic_days_);
  auto const& service2 = rsb_.origin_services_[1];
  ASSERT_EQ(bitfield{"0000010"}, service2->traffic_days_);
  auto const& service3 = rsb_.origin_services_[2];
  ASSERT_EQ(bitfield{"0000100"}, service3->traffic_days_);

  // check rule services
  ASSERT_EQ(3, rsb_.rule_services_.size());

  auto const& rule_service1 = rsb_.rule_services_[0];
  ASSERT_EQ(3, rule_service1.services.size());
  ASSERT_EQ(2, rule_service1.rules.size());
  for (auto const& sr : rule_service1.rules) {
    ASSERT_EQ(RuleType_THROUGH, sr.rule_info.type);
    ASSERT_EQ(bitfield{"0100000"}, sr.s1->traffic_days_);
    ASSERT_EQ(bitfield{"0100000"}, sr.s2->traffic_days_);
  }

  auto const& rule_service2 = rsb_.rule_services_[1];
  ASSERT_EQ(2, rule_service2.services.size());
  ASSERT_EQ(1, rule_service2.rules.size());
  for (auto const& sr : rule_service2.rules) {
    ASSERT_EQ(RuleType_THROUGH, sr.rule_info.type);
    ASSERT_EQ(bitfield{"0001000"}, sr.s1->traffic_days_);
    ASSERT_EQ(bitfield{"0001000"}, sr.s2->traffic_days_);
  }

  auto const& rule_service3 = rsb_.rule_services_[2];
  ASSERT_EQ(2, rule_service3.services.size());
  ASSERT_EQ(1, rule_service3.rules.size());
  for (auto const& sr : rule_service3.rules) {
    ASSERT_EQ(RuleType_THROUGH, sr.rule_info.type);
    ASSERT_EQ(bitfield{"0010000"}, sr.s1->traffic_days_);
    ASSERT_EQ(bitfield{"0010000"}, sr.s2->traffic_days_);
  }
}

TEST_F(loader_ts_2_to_1, rule_services) {
  // check remaining services
  ASSERT_EQ(0, rsb_.origin_services_.size());

  // check rule services
  ASSERT_EQ(2, rsb_.rule_services_.size());

  auto const& rule_service1 = rsb_.rule_services_[0];
  ASSERT_EQ(2, rule_service1.services.size());
  ASSERT_EQ(1, rule_service1.rules.size());
  for (auto const& sr : rule_service1.rules) {
    ASSERT_EQ(RuleType_THROUGH, sr.rule_info.type);
    ASSERT_EQ(bitfield{"0011111"}, sr.s1->traffic_days_);
    ASSERT_EQ(bitfield{"0011111"}, sr.s2->traffic_days_);
  }

  auto const& rule_service2 = rsb_.rule_services_[1];
  ASSERT_EQ(2, rule_service2.services.size());
  ASSERT_EQ(1, rule_service2.rules.size());
  for (auto const& sr : rule_service2.rules) {
    ASSERT_EQ(RuleType_THROUGH, sr.rule_info.type);
    ASSERT_EQ(bitfield{"1100000"}, sr.s1->traffic_days_);
    ASSERT_EQ(bitfield{"1100000"}, sr.s2->traffic_days_);
  }
}

TEST_F(loader_ts_2_to_1_cycle, rule_services) {
  // check remaining services
  ASSERT_EQ(0, rsb_.origin_services_.size());

  // check rule services
  ASSERT_EQ(1, rsb_.rule_services_.size());

  auto const& rule_service1 = rsb_.rule_services_[0];
  ASSERT_EQ(4, rule_service1.services.size());
  ASSERT_EQ(3, rule_service1.rules.size());
  for (auto const& sr : rule_service1.rules) {
    ASSERT_EQ(RuleType_THROUGH, sr.rule_info.type);
    ASSERT_EQ(bitfield{"1111111"}, sr.s1->traffic_days_);
    ASSERT_EQ(bitfield{"1111111"}, sr.s2->traffic_days_);
  }
}

TEST_F(loader_ts_twice_2_to_1_cycle, rule_services) {
  // check remaining services
  ASSERT_EQ(0, rsb_.origin_services_.size());

  // check rule services
  ASSERT_EQ(1, rsb_.rule_services_.size());

  auto const& rule_service1 = rsb_.rule_services_[0];
  ASSERT_EQ(5, rule_service1.services.size());
  ASSERT_EQ(4, rule_service1.rules.size());
  for (auto const& sr : rule_service1.rules) {
    ASSERT_EQ(RuleType_THROUGH, sr.rule_info.type);
    ASSERT_EQ(bitfield{"1111111"}, sr.s1->traffic_days_);
    ASSERT_EQ(bitfield{"1111111"}, sr.s2->traffic_days_);
  }
}

TEST_F(loader_ts_passing_service, rule_services) {
  // check remaining services
  ASSERT_EQ(1, rsb_.origin_services_.size());

  auto const& remaining_service = rsb_.origin_services_[0];
  ASSERT_EQ(bitfield{"1100000"}, remaining_service->traffic_days_);

  // check rule services
  ASSERT_EQ(1, rsb_.rule_services_.size());

  auto const& rule_service = rsb_.rule_services_[0];
  ASSERT_EQ(2, rule_service.services.size());
  ASSERT_EQ(1, rule_service.rules.size());
  for (auto const& sr : rule_service.rules) {
    ASSERT_EQ(RuleType_THROUGH, sr.rule_info.type);
    ASSERT_EQ(bitfield{"0011111"}, sr.s1->traffic_days_);
    ASSERT_EQ(bitfield{"0011111"}, sr.s2->traffic_days_);
  }
}

TEST_F(loader_mss_once, rule_services) {
  // check remaining services
  ASSERT_EQ(1, rsb_.origin_services_.size());

  auto const& remaining_service = rsb_.origin_services_[0].get();
  ASSERT_EQ(bitfield{"1111011"}, remaining_service->traffic_days_);

  // check rule services
  ASSERT_EQ(1, rsb_.rule_services_.size());

  auto const& rule_service = rsb_.rule_services_[0];
  for (auto const& sr : rule_service.rules) {
    ASSERT_EQ(RuleType_MERGE_SPLIT, sr.rule_info.type);
    ASSERT_EQ(bitfield{"0000100"}, sr.s1->traffic_days_);
    ASSERT_EQ(bitfield{"0000100"}, sr.s2->traffic_days_);
  }
}

TEST_F(loader_mss_twice, rule_services) {
  // check remaining services
  ASSERT_EQ(0, rsb_.origin_services_.size());

  // check rule services
  ASSERT_EQ(1, rsb_.rule_services_.size());

  auto const& rule_service = rsb_.rule_services_[0];
  ASSERT_EQ(3, rule_service.services.size());
  ASSERT_EQ(2, rule_service.rules.size());
  for (auto const& sr : rule_service.rules) {
    ASSERT_EQ(RuleType_MERGE_SPLIT, sr.rule_info.type);
    ASSERT_EQ(bitfield{"1111111"}, sr.s1->traffic_days_);
    ASSERT_EQ(bitfield{"1111111"}, sr.s2->traffic_days_);
  }
}

TEST_F(loader_mss_many, rule_services) {
  // check remaining services
  ASSERT_EQ(0, rsb_.origin_services_.size());

  // check rule services
  ASSERT_EQ(1, rsb_.rule_services_.size());

  auto const& rule_service = rsb_.rule_services_[0];
  ASSERT_EQ(3, rule_service.services.size());
  ASSERT_EQ(2, rule_service.rules.size());
  for (auto const& sr : rule_service.rules) {
    ASSERT_EQ(RuleType_MERGE_SPLIT, sr.rule_info.type);
    ASSERT_EQ(bitfield{"1111111"}, sr.s1->traffic_days_);
    ASSERT_EQ(bitfield{"1111111"}, sr.s2->traffic_days_);
  }
}

}  // loader
}  // motis
}  // hrd