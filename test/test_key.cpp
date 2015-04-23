#include <cmath>

#include <boost/test/unit_test.hpp>

#include "she.hpp"
#include "exceptions.hpp"
#include "serialization_formats.hpp"

using std::abs;
using std::stringstream;
using std::vector;

using she::precondition_not_satisfied;
using she::ParameterSet;
using she::PrivateKey;


BOOST_AUTO_TEST_SUITE(ParameterSetSuite)


BOOST_AUTO_TEST_CASE(parameter_set_construction)
{
    {
        const ParameterSet params { 62, 100, 1000, 100000, 5 };
        BOOST_CHECK_EQUAL(params.security, 62);
        BOOST_CHECK_EQUAL(params.noise_size_bits, 100);
        BOOST_CHECK_EQUAL(params.private_key_size_bits, 1000);
        BOOST_CHECK_EQUAL(params.ciphertext_size_bits, 100000);
        BOOST_CHECK_EQUAL(params.oracle_seed, 5);
    }

    {
        BOOST_CHECK_THROW(ParameterSet(62, 1000, 100, 10000, 5), precondition_not_satisfied);
        BOOST_CHECK_THROW(ParameterSet(62, 100, 1000, 999, 5), precondition_not_satisfied);
        BOOST_CHECK_THROW(ParameterSet(62, 0, 1, 2, 5), precondition_not_satisfied);
    }
}

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

    BOOST_CHECK_GT(
      params.degree() - 1,
      circuit_mult_size
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

BOOST_AUTO_TEST_CASE_TEMPLATE(parameter_set_serialization, Format, Formats)
{
    const ParameterSet params { 62, 100, 1000, 100000, 5 };
    ParameterSet restored_params {};

    stringstream ss;
    {
        typename Format::oarchive oa(ss);
        oa << params;
    }
    {
        typename Format::iarchive ia(ss);
        ia >> restored_params;
    }

    BOOST_CHECK(params == restored_params);
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(PrivateKeySuite)

BOOST_AUTO_TEST_CASE(private_key_construction_accessors_and_comparison)
{
    const auto params = ParameterSet::generate_parameter_set(62, 10, 42);
    const PrivateKey sk(params);

    BOOST_CHECK(sk.parameter_set() == params);
    const auto private_element_size = mpz_sizeinbase(sk.private_element().get_mpz_t(), 2);
    BOOST_CHECK_EQUAL(private_element_size, params.private_key_size_bits);

    // These should not be the same, because new private key elements are generated every time
    const PrivateKey other_sk(params);
    BOOST_CHECK(sk != other_sk);
    BOOST_CHECK(!(sk == other_sk));
}

BOOST_AUTO_TEST_CASE(private_key_encryption_decryption)
{
    const auto params = ParameterSet::generate_parameter_set(62, 5, 42);
    const PrivateKey sk(params);

    const size_t iterations = 15;

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

BOOST_AUTO_TEST_CASE_TEMPLATE(private_key_serialization, Format, Formats)
{
    const auto params = ParameterSet::generate_parameter_set(62, 5, 42);
    const PrivateKey sk(params);
    PrivateKey restored_sk;

    stringstream ss;
    {
        typename Format::oarchive oa(ss);
        oa << BOOST_SERIALIZATION_NVP(sk);
    }
    {
        typename Format::iarchive ia(ss);
        ia >> BOOST_SERIALIZATION_NVP(restored_sk);
    }

    BOOST_CHECK(sk == restored_sk);
}

BOOST_AUTO_TEST_SUITE_END()
