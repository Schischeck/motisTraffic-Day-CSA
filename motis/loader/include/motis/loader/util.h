#pragma once

#include <cmath>
#include <cinttypes>
#include <string>
#include <bitset>
#include <algorithm>
#include <memory>
#include <map>

#include "boost/filesystem.hpp"

#include "flatbuffers/flatbuffers.h"

#include "parser/cstr.h"
#include "parser/file.h"

namespace motis {
namespace loader {

template <typename T>
inline flatbuffers::Offset<flatbuffers::String> to_fbs_string(
    flatbuffers::FlatBufferBuilder& b, T const& s) {
  return b.CreateString(s.c_str(), s.length());
}

template <typename T>
flatbuffers::Offset<flatbuffers::String> to_fbs_string(
    flatbuffers::FlatBufferBuilder& b, T const& s,
    std::string const& /* charset -> currently only supported: ISO-8859-1 */) {
  std::vector<unsigned char> utf8(s.length() * 2, '\0');
  auto number_of_input_bytes = s.length();
  unsigned char const* in = reinterpret_cast<unsigned char const*>(s.c_str());
  unsigned char* out_begin = &utf8[0];
  unsigned char* out = out_begin;
  for (std::size_t i = 0; i < number_of_input_bytes; ++i) {
    if (*in < 128) {
      *out++ = *in++;
    } else {
      *out++ = 0xc2 + (*in > 0xbf);
      *out++ = (*in++ & 0x3f) + 0x80;
    }
  }
  return to_fbs_string(b, parser::cstr(reinterpret_cast<char const*>(out_begin),
                                       std::distance(out_begin, out)));
}

template <typename IndexType, typename ValueType>
inline std::vector<ValueType> values(std::map<IndexType, ValueType> const& m) {
  std::vector<ValueType> v(m.size());
  std::transform(begin(m), end(m), begin(v),
                 [](std::pair<IndexType, ValueType> const& entry) {
                   return entry.second;
                 });
  return v;
}

template <typename IntType>
inline IntType raw_to_int(parser::cstr s) {
  IntType key = 0;
  std::memcpy(&key, s.str, std::min(s.len, sizeof(IntType)));
  return key;
}

template <typename It, typename Predicate>
inline It find_nth(It begin, It end, std::size_t n, Predicate fun) {
  assert(n != 0);
  std::size_t num_elements_found = 0;
  auto it = begin;
  while (it != end && num_elements_found != n) {
    it = std::find_if(it, end, fun);
    ++num_elements_found;
    if (num_elements_found != n) {
      ++it;
    }
  }
  return it;
}

template <typename K, typename V, typename CreateFun>
V& get_or_create(std::map<K, V>& m, K const& key, CreateFun f) {
  auto it = m.find(key);
  if (it != end(m)) {
    return it->second;
  } else {
    return m[key] = f();
  }
}

template <typename TargetCollection, typename It, typename UnaryOperation>
inline TargetCollection transform(It s, It e, UnaryOperation op) {
  TargetCollection c;
  std::transform(s, e, std::back_insert_iterator<TargetCollection>(c), op);
  return c;
}

template <typename It, typename UnaryOperation>
inline auto transform_to_vec(It s, It e, UnaryOperation op)
    -> std::vector<decltype(op(*s))> {
  std::vector<decltype(op(*s))> vec(std::distance(s, e));
  std::transform(s, e, std::begin(vec), op);
  return vec;
}

template <typename T, typename... Args>
std::unique_ptr<T> make_unique_helper(std::false_type, Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

inline int yyyymmdd_year(int yyyymmdd) { return yyyymmdd / 10000; }
inline int yyyymmdd_month(int yyyymmdd) { return (yyyymmdd % 10000) / 100; }
inline int yyyymmdd_day(int yyyymmdd) { return yyyymmdd % 100; }

#if !defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__)
template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
  return make_unique_helper<T>(std::is_array<T>(), std::forward<Args>(args)...);
}
#else
using std::make_unique;
#endif

parser::buffer load_file(boost::filesystem::path const&);

int hhmm_to_min(int hhmm);

void write_schedule(flatbuffers::FlatBufferBuilder& b,
                    boost::filesystem::path const& path);

void collect_files(boost::filesystem::path const& root,
                   std::vector<boost::filesystem::path>& files);

}  // namespace loader
}  // namespace motis
