#pragma once

#include "flatbuffers/flatbuffers.h"

#include "motis/protocol/Message_generated.h"

namespace flatbuffers {
struct Parser;
}

namespace motis {
namespace module {

struct message {
  message() : msg_(nullptr) {}
  message(std::string const& json);
  message(flatbuffers::unique_ptr_t mem, Message const* msg, void* buf)
      : mem_(std::move(mem)), msg_(msg), buf_(buf) {}

  operator bool() { return msg_ != nullptr; }

  std::string to_json() const;

  Message const& operator->() const { return *msg_; }

  static void init_parser();

  static std::unique_ptr<flatbuffers::Parser> parser;
  flatbuffers::unique_ptr_t mem_;
  Message const* msg_;
  void* buf_;
};

typedef std::shared_ptr<message> msg_ptr;

inline msg_ptr make_msg(flatbuffers::FlatBufferBuilder& builder) {
  auto buf = builder.GetBufferPointer();
  auto msg = GetMessage(buf);
  auto mem = builder.ReleaseBufferPointer();
  builder.Clear();
  return std::make_shared<message>(std::move(mem), msg, buf);
}

inline msg_ptr make_msg(std::string const& json) {
  return std::make_shared<message>(json);
}

}  // namespace module
}  // namespace motis
