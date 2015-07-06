#include "boost/test/unit_test.hpp"

namespace motis {
namespace railviz {
namespace example_test_module {

struct Fixure
{
    Fixure()
    {
    }
};

BOOST_FIXTURE_TEST_SUITE( example_suit, Fixure )

BOOST_AUTO_TEST_CASE( example_test )
{
}

BOOST_AUTO_TEST_SUITE_END()

}
}
}
