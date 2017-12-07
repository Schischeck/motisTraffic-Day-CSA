#include "motis/ris/ris.h"

#include <atomic>
#include <optional>

#include "boost/filesystem.hpp"

#include "conf/date_time.h"
#include "conf/simple_config_param.h"

#include "tar/file_reader.h"
#include "tar/tar_reader.h"
#include "tar/zstd_reader.h"

#include "lmdb/lmdb.hpp"

#include "motis/core/common/logging.h"
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
//        earliest <= day.begin && latest >= day.end
constexpr auto const MIN_DAY_DB = "MIN_DAY_DB";

// index for every day referenced by any message
// key: day.begin (unix timestamp)
// value: largest message timestamp from MSG_DB that has
//        earliest <= day.begin && latest >= day.end
constexpr auto const MAX_DAY_DB = "MAX_DAY_DB";

struct ris::impl {
  void init() {
    env_.set_maxdbs(4);
    env_.set_mapsize(static_cast<uint64_t>(1024) * 1024 * 1024 * 10);
    env_.open(db_path_.c_str(),
              db::env_open_flags::NOMEMINIT | lmdb::env_open_flags::NOSUBDIR);

    db::txn t{env_};
    t.dbi_open(FILE_DB, db::dbi_flags::CREATE);
    t.dbi_open(MSG_DB, db::dbi_flags::CREATE | db::dbi_flags::INTEGERKEY);
    t.dbi_open(MIN_DAY_DB, db::dbi_flags::CREATE | db::dbi_flags::INTEGERKEY);
    t.dbi_open(MAX_DAY_DB, db::dbi_flags::CREATE | db::dbi_flags::INTEGERKEY);
    t.commit();

    if (fs::is_directory(input_folder_)) {
      parse_folder(input_folder_);
      env_.force_sync();
    } else {
      LOG(warn) << input_folder_ << " is not a directory, skipping";
    }
    // forward(new_time=init_time_) TODO(felix)
  }

  msg_ptr upload(msg_ptr const&) {
    // TODO(felix)
    // write to folder()
    // parse_folder()
    // forward()
    return {};
  }

  msg_ptr forward(msg_ptr const&) {
    // TODO(felix)
    /*
      cursor c;
      std::vector<parser::buffer> blobs;
      for (auto const& m : c) {
        if (m->timstamp() > batch_to) {
          publish(blobs);
          blobs.clear();
        }
        blobs.emplace_back(m);
      }
    */
    return {};
  }

  std::string db_path_;
  std::string input_folder_;
  conf::holder<std::time_t> init_time_{0};

private:
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

  void parse_folder(fs::path const& p) {
    auto const root = p.root_path();
    std::vector<std::shared_ptr<ctx::future<ctx_data, void>>> futures;
    for (auto const& it : fs::directory_iterator(p)) {
      auto const& rel_path = it.path();
      if (fs::is_regular_file(rel_path)) {
        auto const p = fs::canonical(rel_path, root);
        if (auto const t = get_file_type(p);
            t != file_type::NONE && !is_known_file(p)) {
          futures.emplace_back(
              spawn_job_void([p, t, this]() { write_to_db(p, t); }));
        }
      } else if (fs::is_directory(rel_path)) {
        parse_folder(rel_path);
      }
    }

    for (auto& f : futures) {
      f->val();
    }
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
    using tar_zst_reader = tar_reader<zstd_reader>;

    auto first = false;
    auto curr_timestamp = time_t{0};
    auto buf = std::vector<char>{};
    auto flush_to_db = [&]() {
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

    auto write = [&](ris_message&& m) {
      if (!first && m.timestamp_ != curr_timestamp) {
        flush_to_db();
      }

      using size_type = uint16_t;
      constexpr auto const size_type_size = sizeof(size_type);

      auto const base = buf.size();
      buf.resize(buf.size() + 2 + m.size());

      auto const msg_size = static_cast<size_type>(m.size());
      std::memcpy(&buf[0] + base, &msg_size, size_type_size);
      std::memcpy(&buf[0] + base + size_type_size, m.data(), m.size());
      curr_timestamp = m.timestamp_;
      first = false;
    };

    printf("\n");
    auto parse = [&](auto&& reader) {
      std::optional<std::string_view> xml;
      unsigned print = 0;
      while ((xml = reader.read())) {
        xml_to_ris_message(*xml, [&](ris_message&& m) { write(std::move(m)); });
        if (++print % 1000) {
          printf("\r%s: %d%%", p.c_str(),
                 static_cast<int>(100 * reader.progress()));
          fflush(stdout);
        }
      }
    };

    auto cp = p.c_str();
    switch (type) {
      case file_type::ZST: parse(tar_zst_reader{zstd_reader{cp}}); break;
      case file_type::ZIP: parse(zip_reader{cp}); break;
      case file_type::XML: parse(file_reader{cp}); break;
      default: assert(false);
    }

    add_to_known_files(p);
  }

  db::env env_;
  std::atomic<uint64_t> next_msg_id_{0};
};

ris::ris() : module("RIS", "ris"), impl_(std::make_unique<impl>()) {
  string_param(impl_->db_path_, "ris.mdb", "db", "ris database path");
  string_param(impl_->input_folder_, "ris", "input_folder", "ris input folder");
  template_param(impl_->init_time_, {0l}, "init_time",
                 "'forward' the simulation clock (expects Unix timestamp)");
}

ris::~ris() = default;

void ris::init(motis::module::registry& r) {
  r.subscribe_void("/init", [this] { impl_->init(); });
  r.register_op("/ris/upload", [this](auto&& m) { return impl_->upload(m); });
  r.register_op("/ris/forward", [this](auto&& m) { return impl_->forward(m); });
}

}  // namespace ris
}  // namespace motis
