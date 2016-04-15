#pragma once

#include <string>
#include <tuple>

#include "libpq-fe.h"

#include "motis/db/connection.h"
#include "motis/db/format.h"
#include "motis/db/nullable.h"
#include "motis/db/result_base.h"

namespace motis {
namespace db {

template <char const* C, typename In, typename Out>
struct prep_stmt;

template <char const* C, typename... InTypes, typename... OutTypes>
struct prep_stmt<C, std::tuple<InTypes...>, std::tuple<OutTypes...>> {
  prep_stmt(connection_handle& conn)
      : conn_(conn), name_(conn.register_prep_stmt(C)) {
    // TODO ??? int PQnparams(const PGresult *res);
  }

  struct result : public result_base {
    static constexpr std::size_t kResultSize = sizeof...(OutTypes);
    using RawTuple = std::tuple<OutTypes...>;
    using NullableTuple = std::tuple<nullable<OutTypes>...>;
    using Indices = std::make_index_sequence<sizeof...(OutTypes)>;

    result(PGresult* res) : result_base(res), current_row_(0) {
      assert(kResultSize == PQnfields(res_));
    }

    int size() const { return PQntuples(res_); }

    NullableTuple get_row(size_t const row) {
      assert(row < size());
      return get_row_helper(row, Indices());
    }

  private:
    template <std::size_t... I>
    NullableTuple get_row_helper(size_t const row, std::index_sequence<I...>) {
      return std::make_tuple(
          bind_result<typename std::tuple_element<I, RawTuple>::type>(row,
                                                                      I)...);
    }

    template <typename T>
    nullable<T> bind_result(size_t const row, std::size_t const column) {
      if (!PQgetisnull(res_, current_row_, column)) {
        return deserialize<T>(PQgetvalue(res_, row, column));
      }
      return {};
    }

  public:
    bool has_next() { return current_row_ < size(); }
    NullableTuple next() {
      assert(current_row_ < size());
      return get_row(current_row_++);
    }

  private:
    std::size_t current_row_;
  };

  result operator()(InTypes&&... in) {
    std::vector<std::string> serialized{serialize(in)...};
    std::vector<char const*> values;
    for (auto const& s : serialized) {
      values.push_back(s.c_str());
    }

    result r(PQexecPrepared(conn_.conn_,  // PGconn *conn
                            name_.c_str(),  // const char *stmtName
                            values.size(),  // int nParams,
                            values.data(),  // const char * const *paramValues
                            nullptr,  // const int *paramLengths
                            nullptr,  // const int *paramFormats
                            0  // int resultFormat
                            ));
    r.ensure_valid();
    return r;
  }

  static result exec(connection_handle& conn, InTypes&&... in) {
    prep_stmt stmt(conn);
    return stmt(std::forward<InTypes>(in)...);
  }

  connection_handle& conn_;
  std::string name_;
};

template <char const* C>
using void_stmt = prep_stmt<C, std::tuple<>, std::tuple<>>;

template <char const* C, typename... InTypes>
using in_stmt = prep_stmt<C, std::tuple<InTypes...>, std::tuple<>>;

template <char const* C, typename... OutTypes>
using out_stmt = prep_stmt<C, std::tuple<>, std::tuple<OutTypes...>>;

}  // namespace db
}  // namespace motis
