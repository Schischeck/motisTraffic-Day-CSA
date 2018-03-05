#include "motis/ris/ris.h"

#include <atomic>
#include <optional>

#include "boost/filesystem.hpp"

#include "utl/concat.h"

#include "conf/date_time.h"
#include "conf/simple_config_param.h"

#include "tar/file_reader.h"
#include "tar/tar_reader.h"
#include "tar/zstd_reader.h"

#include "lmdb/lmdb.hpp"

#include "motis/core/common/logging.h"
#include "motis/module/context/get_schedule.h"
#include "motis/module/context/motis_publish.h"
#include "motis/module/context/motis_spawn.h"
#include "motis/ris/risml/risml_parser.h"
#include "motis/ris/zip_reader.h"

namespace fs = boost::filesystem;
namespace db = lmdb;
using namespace motis::module;
using namespace motis::logging;
using motis::ris::risml::xml_to_ris_message;
using tar::file_reader;
using tar::tar_reader;
using tar::zstd_reader;

namespace motis {
namespace ris {

// stores the list of files that were already parsed
// key: path
// value: empty
constexpr auto const FILE_DB = "FILE_DB";

// messages, no specific order (unique id)
// key: timestamp
// value: buffer of messages:
//        2 bytes message size, {message size} bytes message
constexpr auto const MSG_DB = "MSG_DB";

// index for every day referenced by any message
// key: day.begin (unix timestamp)
// value: smallest message timestamp from MSG_DB that has
//        earliest <= day.end && latest >= day.begin
constexpr auto const MIN_DAY_DB = "MIN_DAY_DB";

// index for every day referenced by any message
// key: day.begin (unix timestamp)
// value: largest message timestamp from MSG_DB that has
//        earliest <= day.end && latest >= day.begin
constexpr auto const MAX_DAY_DB = "MAX_DAY_DB";

constexpr auto const BATCH_SIZE = time_t{3600};

constexpr auto const BUCKET_SIZE = time_t{60 * 10};

constexpr bool is_same_bucket(time_t const t1, time_t const t2) {
  return t1 / BUCKET_SIZE == t2 / BUCKET_SIZE;
}

using size_type = uint16_t;
constexpr auto const SIZE_TYPE_SIZE = sizeof(size_type);

constexpr auto const SECONDS_A_DAY = time_t{24 * 60 * 60};
constexpr time_t day(time_t t) { return (t / SECONDS_A_DAY) * SECONDS_A_DAY; }
constexpr time_t next_day(time_t t) { return day(t) + SECONDS_A_DAY; }

template <typename Fn>
inline void for_each_day(ris_message const& m, Fn&& f) {
  auto const last = next_day(m.latest_);
  for (auto d = day(m.earliest_); d != last; d += SECONDS_A_DAY) {
    f(d);
  }
}

template <typename T>
constexpr T floor(T const i, T const multiple) {
  return (i / multiple) * multiple;
}

template <typename T>
constexpr T ceil(T const i, T const multiple) {
  return ((i - 1) / multiple) * multiple + multiple;
}

time_t to_time_t(std::string_view s) {
  return *reinterpret_cast<time_t const*>(s.data());
}

std::string_view from_time_t(time_t const& t) {
  return {reinterpret_cast<char const*>(&t), sizeof(t)};
}

struct ris::impl {
  void init() {
    if (clear_db_) {
      LOG(info) << "clearing database path " << db_path_;
      fs::remove_all(db_path_);
    }

    env_.set_maxdbs(4);
    env_.set_mapsize(db_max_size_);
    env_.open(db_path_.c_str(),
              lmdb::env_open_flags::NOSUBDIR | lmdb::env_open_flags::NOSYNC);

    db::txn t{env_};
    t.dbi_open(FILE_DB, db::dbi_flags::CREATE);
    t.dbi_open(MSG_DB, db::dbi_flags::CREATE | db::dbi_flags::INTEGERKEY);
    t.dbi_open(MIN_DAY_DB, db::dbi_flags::CREATE | db::dbi_flags::INTEGERKEY);
    t.dbi_open(MAX_DAY_DB, db::dbi_flags::CREATE | db::dbi_flags::INTEGERKEY);
    t.commit();

    if (fs::exists(input_)) {
      LOG(warn) << "parsing " << input_;
      parse(input_);
      env_.force_sync();
      forward(init_time_);
    } else {
      LOG(warn) << input_ << " does not exist";
      return;
    }
  }

  msg_ptr upload(msg_ptr const& msg) {
    auto const req = motis_content(HTTPRequest, msg);

    write_to_db(zip_reader{req->content()->c_str(), req->content()->size()});

    message_creator fbb;
    std::vector<flatbuffers::Offset<MessageHolder>> offsets;

    zip_reader reader{req->content()->c_str(), req->content()->size()};
    std::optional<std::string_view> xml;
    while ((xml = reader.read())) {
      xml_to_ris_message(*xml, [&](ris_message&& m) {
        offsets.push_back(
            CreateMessageHolder(fbb, fbb.CreateVector(m.data(), m.size())));
      });
    }

    fbb.create_and_finish(
        MsgContent_RISBatch,
        CreateRISBatch(fbb, fbb.CreateVector(offsets)).Union(),
        "/ris/messages");

    ctx::await_all(motis_publish(make_msg(fbb)));

    return {};
  }

  msg_ptr forward(msg_ptr const& msg) {
    forward(motis_content(RISForwardTimeRequest, msg)->new_time());
    return {};
  }

  std::string db_path_;
  std::string input_;
  conf::holder<std::time_t> init_time_{0};
  bool clear_db_ = false;
  size_t db_max_size_{static_cast<size_t>(1024) * 1024 * 1024 * 512};

private:
  void forward(time_t const to) {
    std::lock_guard<std::mutex> lock{forward_mutex_};
    auto const& sched = get_schedule();
    auto const first_schedule_event_day =
        floor(sched.first_event_schedule_time_, SECONDS_A_DAY);
    auto const last_schedule_event_day =
        ceil(sched.last_event_schedule_time_, SECONDS_A_DAY);
    auto const min_timestamp =
        get_min_timestamp(first_schedule_event_day, last_schedule_event_day);
    if (min_timestamp) {
      forward(std::max(*min_timestamp, sched.system_time_), to);
    } else {
      LOG(info) << "ris database has no relevant data";
    }
  }

  void forward(time_t const from, time_t const to) {
    LOG(info) << "forwarding from " << logging::time(from) << " to "
              << logging::time(to);

    message_creator fbb;
    std::vector<flatbuffers::Offset<MessageHolder>> offsets;
    auto flush = [&]() {
      if (offsets.empty()) {
        return;
      }

      fbb.create_and_finish(
          MsgContent_RISBatch,
          CreateRISBatch(fbb, fbb.CreateVector(offsets)).Union(),
          "/ris/messages");

      ctx::await_all(motis_publish(make_msg(fbb)));

      fbb.Clear();
      offsets.clear();
    };

    auto const& sched = get_schedule();

    auto const from_bucket = floor(from, BUCKET_SIZE);
    auto const to_bucket = ceil(to, BUCKET_SIZE);

    auto t = db::txn{env_, db::txn_flags::RDONLY};
    auto db = t.dbi_open(MSG_DB);
    auto c = db::cursor{t, db};
    auto bucket = c.get(db::cursor_op::SET_RANGE, from_bucket);
    auto batch_begin = bucket ? bucket->first : 0;
    while (true) {
      if (!bucket) {
        break;
      }

      auto const & [ timestamp, msgs ] = *bucket;
      if (timestamp >= to_bucket) {
        break;
      }

      auto ptr = msgs.data();
      auto const end = ptr + msgs.size();
      while (ptr < end) {
        size_type size;
        std::memcpy(&size, ptr, SIZE_TYPE_SIZE);
        ptr += SIZE_TYPE_SIZE;

        if (auto const msg = GetMessage(ptr);
            msg->timestamp() < to && msg->timestamp() >= from &&
            msg->earliest() <= sched.last_event_schedule_time_ &&
            msg->latest() >= sched.first_event_schedule_time_) {
          offsets.push_back(CreateMessageHolder(
              fbb,
              fbb.CreateVector(reinterpret_cast<uint8_t const*>(ptr), size)));
        }

        ptr += size;
      }

      if (timestamp - batch_begin > BATCH_SIZE) {
        flush();
        batch_begin = timestamp;
      }

      bucket = c.get(db::cursor_op::NEXT, 0);
    }

    flush();
    get_schedule().system_time_ = to;
    motis_publish(make_no_msg("/ris/system_time_changed"));
  }

  std::optional<time_t> get_min_timestamp(time_t const from_day,
                                          time_t const to_day) {
    verify(from_day % SECONDS_A_DAY == 0, "from not a day");
    verify(to_day % SECONDS_A_DAY == 0, "to not a day");

    constexpr auto const max = std::numeric_limits<time_t>::max();

    auto min = max;
    auto t = db::txn{env_, db::txn_flags::RDONLY};
    auto db = t.dbi_open(MIN_DAY_DB);
    for (auto d = from_day; d != to_day; d += SECONDS_A_DAY) {
      auto const r = t.get(db, d);
      if (r) {
        min = std::min(min, to_time_t(*r));
      }
    }

    return min != max ? std::make_optional(min) : std::nullopt;
  }

  enum class file_type { NONE, ZST, ZIP, XML };

  static file_type get_file_type(fs::path const& p) {
    if (p.extension() == ".zst") {
      return file_type::ZST;
    } else if (p.extension() == ".zip") {
      return file_type::ZIP;
    } else if (p.extension() == ".xml") {
      return file_type::XML;
    } else {
      return file_type::NONE;
    }
  }

  std::vector<std::pair<fs::path, file_type>> collect_files(fs::path const& p) {
    if (fs::is_regular_file(p)) {
      if (auto const t = get_file_type(p);
          t != file_type::NONE && !is_known_file(p)) {
        return {std::make_pair(p, t)};
      }
    } else if (fs::is_directory(p)) {
      std::vector<std::pair<fs::path, file_type>> files;
      for (auto const& entry : fs::directory_iterator(p)) {
        utl::concat(files, collect_files(entry));
      }
      return files;
    }
    return {};
  }

  void parse(fs::path const& p) {
    ctx::await_all(utl::to_vec(
        collect_files(fs::canonical(p, p.root_path())), [&](auto&& e) {
          return spawn_job_void(
              [e, this]() { write_to_db(e.first, e.second); });
        }));
  }

  bool is_known_file(fs::path const& p) {
    auto t = db::txn{env_};
    auto db = t.dbi_open(FILE_DB);
    return t.get(db, p.generic_string()).has_value();
  }

  void add_to_known_files(fs::path const& p) {
    auto t = db::txn{env_};
    auto db = t.dbi_open(FILE_DB);
    t.put(db, p.generic_string(), "");
    t.commit();
  }

  void write_to_db(fs::path const& p, file_type const type) {
    using tar_zst = tar_reader<zstd_reader>;
    auto const& cp = p.generic_string();
    switch (type) {
      case file_type::ZST: write_to_db(tar_zst(zstd_reader(cp.c_str()))); break;
      case file_type::ZIP: write_to_db(zip_reader(cp.c_str())); break;
      case file_type::XML: write_to_db(file_reader(cp.c_str())); break;
      default: assert(false);
    }
    add_to_known_files(p);
  }

  template <typename Reader>
  void write_to_db(Reader&& reader) {
    std::map<time_t /* d.b */, time_t /* min(t) : e <= d.e && l >= d.b */> min;
    std::map<time_t /* d.b */, time_t /* max(t) : e <= d.e && l >= d.b */> max;
    auto curr_timestamp = time_t{0};
    auto buf = std::vector<char>{};
    auto flush_to_db = [&]() {
      if (buf.empty()) {
        return;
      }

      std::lock_guard<std::mutex> lock{merge_mutex_};

      auto t = db::txn{env_};
      auto db = t.dbi_open(MSG_DB);
      auto c = db::cursor{t, db};

      auto const v = c.get(lmdb::cursor_op::SET_RANGE, curr_timestamp);
      if (v && v->first == curr_timestamp) {
        buf.insert(end(buf), begin(v->second), end(v->second));
      }
      c.put(curr_timestamp, std::string_view{&buf[0], buf.size()});
      c.commit();
      t.commit();

      buf.clear();
    };

    auto first = false;
    auto write = [&](ris_message&& m) {
      if (!first && !is_same_bucket(m.timestamp_, curr_timestamp)) {
        flush_to_db();
      }

      auto const base = buf.size();
      buf.resize(buf.size() + 2 + m.size());

      auto const msg_size = static_cast<size_type>(m.size());
      std::memcpy(&buf[0] + base, &msg_size, SIZE_TYPE_SIZE);
      std::memcpy(&buf[0] + base + SIZE_TYPE_SIZE, m.data(), m.size());
      curr_timestamp = m.timestamp_;
      first = false;

      for_each_day(m, [&](time_t const d) {
        if (auto it = min.lower_bound(d); it != end(min) && it->first == d) {
          it->second = std::min(it->second, m.timestamp_);
        } else {
          min.emplace_hint(it, d, m.timestamp_);
        }

        if (auto it = max.lower_bound(d); it != end(max) && it->first == d) {
          it->second = std::max(it->second, m.timestamp_);
        } else {
          max.emplace_hint(it, d, m.timestamp_);
        }
      });
    };

    std::optional<std::string_view> xml;
    while ((xml = reader.read())) {
      xml_to_ris_message(*xml, [&](ris_message&& m) { write(std::move(m)); });
    }

    flush_to_db();
    update_min_max(min, max);
  }

  void update_min_max(std::map<time_t, time_t> const& min,
                      std::map<time_t, time_t> const& max) {
    std::lock_guard<std::mutex> lock{min_max_mutex_};

    auto t = db::txn{env_};
    auto min_db = t.dbi_open(MIN_DAY_DB);
    auto max_db = t.dbi_open(MAX_DAY_DB);

    for (auto const[day, min_timestamp] : min) {
      auto smallest = min_timestamp;
      if (auto entry = t.get(min_db, day); entry) {
        smallest = std::min(smallest, to_time_t(*entry));
      }
      t.put(min_db, day, from_time_t(smallest));
    }

    for (auto const[day, max_timestamp] : max) {
      auto largest = max_timestamp;
      if (auto entry = t.get(max_db, day); entry) {
        largest = std::max(largest, to_time_t(*entry));
      }
      t.put(max_db, day, from_time_t(largest));
    }

    t.commit();
  }

  db::env env_;
  std::atomic<uint64_t> next_msg_id_{0};
  std::mutex min_max_mutex_;
  std::mutex forward_mutex_;
  std::mutex merge_mutex_;
};  // namespace ris

ris::ris() : module("RIS", "ris"), impl_(std::make_unique<impl>()) {
  string_param(impl_->db_path_, "ris.mdb", "db", "ris database path");
  string_param(impl_->input_, "ris", "input", "ris input (folder or risml)");
  size_t_param(impl_->db_max_size_, "db_max_size", "virtual memory map size");
  template_param(impl_->init_time_, "init_time", "initial forward time");
  bool_param(impl_->clear_db_, "clear_db", "clean db before init");
}

ris::~ris() = default;

void ris::init(motis::module::registry& r) {
  r.subscribe_void("/init", [this] { impl_->init(); });
  r.register_op("/ris/upload", [this](auto&& m) { return impl_->upload(m); });
  r.register_op("/ris/forward", [this](auto&& m) { return impl_->forward(m); });
}

}  // namespace ris
}  // namespace motis
