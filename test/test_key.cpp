#include <cmath>

#include <boost/test/unit_test.hpp>

#include "she.hpp"
#include "serialization_formats.hpp"

using std::stringstream;
using std::vector;

using she::ParameterSet;
using she::PrivateKey;


BOOST_AUTO_TEST_SUITE(ParameterSetSuite)

BOOST_AUTO_TEST_CASE(parameter_set_generation)
{
    const unsigned int security = 62;
    const unsigned int circuit_mult_size = 20;
    const ParameterSet params = ParameterSet::generate_parameter_set(security, circuit_mult_size, 42);

    BOOST_CHECK_EQUAL(
      params.security,
      security
    );

    BOOST_CHECK_EQUAL(
      params.noise_size_bits,
      2 * security
    );

    BOOST_CHECK_EQUAL(
      params.private_key_size_bits,
      security * security + security * circuit_mult_size
    );

    BOOST_CHECK_EQUAL(
      params.ciphertext_size_bits,
      params.private_key_size_bits * params.private_key_size_bits * circuit_mult_size
    );
}

BOOST_AUTO_TEST_CASE(parameter_set_equality_comparison)
{
    const ParameterSet a { 62, 100, 1000, 100000, 5 };
    const ParameterSet b { 62, 100, 1000, 100000, 5 };
    const ParameterSet c { 72, 100, 1000, 100000, 5 };

    BOOST_CHECK(a == b);
    BOOST_CHECK(b != c);
    BOOST_CHECK(c != a);
}

// BOOST_AUTO_TEST_CASE_TEMPLATE(parameter_set_serialization, Format, Formats)
// {
//     const ParameterSet params { 62, 100, 1000, 100000, 5 };
//     ParameterSet restored_params {};
//
//     stringstream ss;
//     {
//         typename Format::oarchive oa(ss);
//         oa << params;
//     }
//     {
//         typename Format::iarchive ia(ss);
//         ia >> restored_params;
//     }
//
//     BOOST_CHECK(params == restored_params);
// }

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(PrivateKeySuite)

BOOST_AUTO_TEST_CASE(private_key_construction_accessors_and_comparison)
{
    const auto params = ParameterSet::generate_parameter_set(62, 10, 42);
    const PrivateKey sk(params);

    BOOST_CHECK(sk.parameter_set() == params);

    const PrivateKey other_sk(params);
    BOOST_CHECK(sk != other_sk);
}

BOOST_AUTO_TEST_CASE(private_key_encryption_decryption)
{
    const auto params = ParameterSet::generate_parameter_set(62, 5, 42);
    const PrivateKey sk(params);

    const size_t iterations = 30;

    const vector<bool> plaintext = {1, 0, 1, 0, 1, 1, 1, 0};
    unsigned int successful_recoveries = 0;
    for (size_t i = 0; i < iterations; ++i)
    {
        const auto ciphertext = sk.encrypt(plaintext);
        const auto restored_plaintext = sk.decrypt(ciphertext.expand());
        if(restored_plaintext == plaintext) {
            ++successful_recoveries;
        }
    }

    BOOST_CHECK_EQUAL(successful_recoveries, iterations);
}

// BOOST_AUTO_TEST_CASE_TEMPLATE(private_key_serialization, Format, Formats)
// {
//     const auto params = ParameterSet::generate_parameter_set(62, 5, 42);
//     const PrivateKey sk(params);
//     PrivateKey restored_sk;
//
//     stringstream ss;
//     {
//         typename Format::oarchive oa(ss);
//         oa << sk;
//     }
//     {
//         typename Format::iarchive ia(ss);
//         ia >> restored_sk;
//     }
//
//     BOOST_CHECK(sk == restored_sk);
// }

BOOST_AUTO_TEST_SUITE_END()
