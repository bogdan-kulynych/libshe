#include <cmath>

#include <boost/test/unit_test.hpp>

#include "she.hpp"
#include "serialization_formats.hpp"

using std::stringstream;
using std::vector;

using she::PlaintextArray;


BOOST_AUTO_TEST_SUITE(PlaintextArraySuite)

BOOST_AUTO_TEST_CASE(plaintext_array_construction_accessors_and_comparison)
{
    const vector<bool> raw_plaintext = {1, 0, 1, 0, 1, 1, 1, 1};

    const PlaintextArray a1(raw_plaintext);

    BOOST_CHECK_EQUAL(a1.degree(), 0);
    BOOST_CHECK_EQUAL(a1.size(), raw_plaintext.size());

    const PlaintextArray a2(raw_plaintext);

    BOOST_CHECK_EQUAL(a2.degree(), 0);

    BOOST_CHECK(a1 == a2);
    BOOST_CHECK(!(a1 != a2));
}

BOOST_AUTO_TEST_CASE(plaintext_array_implicit_conversions)
{
    vector<bool> raw_plaintext = { 1, 0, 1, 0 };

    const PlaintextArray array = raw_plaintext;
    BOOST_CHECK(array == raw_plaintext);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(plaintext_array_serialization, Format, Formats)
{
    const vector<bool> plaintext = {1, 0, 1, 0, 1, 1, 1, 1};
    const PlaintextArray array(plaintext);

    PlaintextArray restored_array;

    // stringstream ss;
    // {
    //     typename Format::oarchive oa(ss);
    //     oa << BOOST_SERIALIZATION_NVP(array);
    // }
    // {
    //     typename Format::iarchive ia(ss);
    //     ia >> BOOST_SERIALIZATION_NVP(restored_array);
    // }

    // BOOST_CHECK(array == restored_array);
}

BOOST_AUTO_TEST_CASE(plaintext_arrays_extend_empty)
{
    PlaintextArray array;
    const auto plaintext = PlaintextArray({1, 1, 1, 1});
    array.extend(plaintext);

    BOOST_CHECK(array.elements() == plaintext.elements());
}

BOOST_AUTO_TEST_CASE(plaintext_arrays_extend)
{
    const auto expected_result = PlaintextArray({1, 1, 0, 0, 0, 0, 1, 1});

    PlaintextArray array({1, 1, 0, 0});
    const auto plaintext = PlaintextArray({0, 0, 1, 1});
    array.extend(plaintext);

    BOOST_CHECK(array.elements() == expected_result.elements());
}

BOOST_AUTO_TEST_CASE(plaintext_arrays_concat)
{
    const vector< vector<bool>> raw_plaintext_inputs = {
        {0, 1, 0, 1},
        {1, 0, 1, 0},
        {0, 0, 0, 0},
        {1, 1, 1, 1}
    };

    vector<PlaintextArray> plaintext_inputs;
    for (const auto & raw_plaintext_input : raw_plaintext_inputs) {
        plaintext_inputs.push_back(PlaintextArray(raw_plaintext_input));
    }

    const auto expected_result = PlaintextArray({0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1});

    BOOST_CHECK(concat(plaintext_inputs) == expected_result);
}

BOOST_AUTO_TEST_SUITE_END()