#include <sstream>

#include <boost/test/unit_test.hpp>

#include <gmpxx.h>

#include "serializations.hpp"
#include "serialization_formats.hpp"

using std::stringstream;


BOOST_AUTO_TEST_SUITE(SerializationsSuite)

BOOST_AUTO_TEST_CASE_TEMPLATE(mpz_class_serialization, Format, Formats)
{
    mpz_class z = mpz_class(1) << 10000;
    mpz_class restored_z;

    stringstream ss;
    {
        typename Format::oarchive oa(ss);
        oa << BOOST_SERIALIZATION_NVP(z);
    }
    {
        typename Format::iarchive ia(ss);
        ia >> BOOST_SERIALIZATION_NVP(restored_z);
    }

    BOOST_CHECK(z == restored_z);
}

BOOST_AUTO_TEST_SUITE_END()