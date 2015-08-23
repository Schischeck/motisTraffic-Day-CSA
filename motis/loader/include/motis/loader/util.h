#pragma once

#include <cmath>
#include <cinttypes>
#include <string>
#include <bitset>
#include <algorithm>
#include <map>

#include "parser/cstr.h"

#include "boost/filesystem/path.hpp"

#include "flatbuffers/flatbuffers.h"

namespace motis {
namespace loader {

template <std::size_t BitSetSize>
struct bitset_comparator {
  bool operator()(std::bitset<BitSetSize> const& lhs,
                  std::bitset<BitSetSize> const& rhs) const {
    for (std::size_t i = 0; i < BitSetSize; ++i) {
      int lhs_bit = lhs.test(i) ? 1 : 0;
      int rhs_bit = rhs.test(i) ? 1 : 0;
      if (lhs_bit != rhs_bit) {
        return lhs_bit < rhs_bit;
      }
    }
    return false;
  }
};

template <std::size_t BitSetSize>
std::bitset<BitSetSize> create_uniform_bitfield(char val) {
  assert(val == '1' || val == '0');

  std::string all_days_bit_str;
  all_days_bit_str.resize(BitSetSize);
  std::fill(begin(all_days_bit_str), end(all_days_bit_str), val);

  return std::bitset<BitSetSize>(all_days_bit_str);
}

void write_schedule(flatbuffers::FlatBufferBuilder& b,
                    boost::filesystem::path const& path);

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

template <int BitCount>
inline std::string serialize_bitset(std::bitset<BitCount> const& bitset) {
  constexpr int number_of_bytes = BitCount >> 3;

  char buf[number_of_bytes];
  std::fill(std::begin(buf), std::end(buf), 0);

  int bit_it = 0;
  for (int byte = 0; byte < number_of_bytes; ++byte) {
    uint8_t next_byte = 0;
    for (int bit = 0; bit < 8; ++bit) {
      uint8_t bit_value = bitset.test(bit_it) ? 1 : 0;
      next_byte += (bit_value << bit);
      ++bit_it;
    }
    buf[byte] = next_byte;
  }

  return std::string(buf, number_of_bytes);
}

template <int BitCount>
inline std::bitset<BitCount> deserialize_bitset(parser::cstr str) {
  constexpr std::size_t number_of_bytes = BitCount >> 3;

  std::bitset<BitCount> bits;
  int bit_it = 0;
  auto limit = std::min(number_of_bytes, str.len);
  for (unsigned byte = 0; byte < limit; ++byte) {
    std::bitset<8> byte_bit_set(str[byte]);
    for (int i = 0; i < 8; ++i) {
      bits.set(bit_it, byte_bit_set.test(i));
      ++bit_it;
    }
  }

  return bits;
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

inline int hhmm_to_min(int hhmm) {
  if (hhmm < 0) {
    return hhmm;
  } else {
    return (hhmm / 100) * 60 + (hhmm % 100);
  }
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

template <typename TargetCollection, typename It, typename UnaryOperation>
inline TargetCollection transform(It begin, It end, UnaryOperation op) {
  TargetCollection c;
  std::transform(begin, end, std::back_insert_iterator<TargetCollection>(c),
                 op);
  return c;
}

template <typename It, typename UnaryOperation>
inline auto transform_to_vec(It s, It e, UnaryOperation op)
    -> std::vector<decltype(op(*s))> {
  using target_collection_t = std::vector<decltype(op(*s))>;
  target_collection_t vec(std::distance(s, e));
  std::transform(s, e, std::begin(vec), op);
  return vec;
}

}  // namespace loader
}  // namespace motis
