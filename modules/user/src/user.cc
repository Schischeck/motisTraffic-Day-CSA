#include "motis/user/user.h"

#include "pgdb_default_conn.h"
#include "pgdb/pgdb.h"

#include "motis/module/message.h"

using namespace motis::module;
using namespace pgdb;

namespace motis {
namespace user {

constexpr char kCreateUser[] = R"sql(
CREATE TABLE IF NOT EXISTS users (
  id BIGSERIAL PRIMARY KEY,
  name text
);
)sql";
using create_users_table = void_stmt<kCreateUser>;

constexpr char kInsertUser[] =
    "INSERT INTO users ( name ) VALUES ( $1 ) RETURNING id;";
using insert_user =
    prep_stmt<kInsertUser, std::tuple<std::string>, std::tuple<long>>;

user::user() : module("User", "user") {
  string_param(conninfo_, PGDB_DEFAULT_CONN, "conninfo",
               "How to connect to a postgres database.");
}

void user::init(registry& r) {
  connection_handle conn(conninfo_);
  create_users_table::exec(conn);

  r.register_op("/user/register",
                [this](msg_ptr const& m) { return register_user(m); });
}

msg_ptr user::register_user(msg_ptr const& msg) {
  auto req = motis_content(UserRegisterRequest, msg);

  auto username = req->name()->str();

  connection_handle conn(conninfo_);
  auto id = single_val<insert_user>(conn, username);

  std::cout << "registered user " << username << " with id " << id << "\n";

  return make_success_msg();
}

}  // namespace user
}  // namespace motis
