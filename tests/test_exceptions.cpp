#include <boost/test/unit_test.hpp>

#include "she/exceptions.hpp"

using she::precondition_not_satisfied;


BOOST_AUTO_TEST_SUITE(ExceptionsSuite)

BOOST_AUTO_TEST_CASE(precondition_not_satisfied_construction_and_accessors)
{
    try {
        ASSERT(true == false, "Obviously not.");
    } catch (const precondition_not_satisfied & e) {
        BOOST_CHECK_EQUAL(e.what(), "Obviously not. (true == false)");
    }
}

BOOST_AUTO_TEST_SUITE_END()

