#include <cmath>
#include <vector>

#include <boost/test/unit_test.hpp>

#include <gmpxx.h>

#include "random.hpp"

using std::vector;
using std::abs;

using she::CSPRNG;
using she::RandomOracle;


BOOST_AUTO_TEST_SUITE(CSPRNG_Suite)

BOOST_AUTO_TEST_CASE(generator_construction)
{
    const CSPRNG generator;
}

BOOST_AUTO_TEST_CASE(generator_get_bits)
{
    const CSPRNG generator;

    const size_t iterations = 15;
    const int bits = 100;

    for (size_t i = 0; i < iterations; ++i) {
        const auto output = generator.get_bits(bits);
        const auto output_bits = static_cast<int>(mpz_sizeinbase(output.get_mpz_t(), 2));
        BOOST_CHECK(abs(output_bits - bits) <= 10);
    }
}

BOOST_AUTO_TEST_CASE(generator_get_range_bits)
{
    const CSPRNG generator;

    const size_t iterations = 30;
    const int bits = 100;

    for (size_t i = 0; i < iterations; ++i) {
        const auto output = generator.get_range_bits(bits);
        const auto output_bits = static_cast<int>(mpz_sizeinbase(output.get_mpz_t(), 2));
        BOOST_CHECK_LT(output_bits, bits + 1);
    }
}

BOOST_AUTO_TEST_CASE(generator_get_range)
{
    const CSPRNG generator;

    const size_t iterations = 15;
    const int bits = 100;
    mpz_class upper_bound = mpz_class(1) << bits;

    for (size_t i = 0; i < iterations; ++i) {
        mpz_class output = generator.get_range(upper_bound);
        BOOST_CHECK(output <= upper_bound);
    }
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(RandomOracleSuite)

BOOST_AUTO_TEST_CASE(oracle_construction)
{
    const RandomOracle oracle(100, 42);
}

BOOST_AUTO_TEST_CASE(oracle_output_generation)
{
    const int bits = 100;
    const unsigned int seed = 42;
    const RandomOracle oracle(bits, seed);
    const auto oracle_outputs = { oracle.next(), oracle.next(), oracle.next() };

    for (const auto & oracle_output : oracle_outputs)
    {
        const auto output_bits = static_cast<int>(mpz_sizeinbase(oracle_output.get_mpz_t(), 2));
        BOOST_CHECK(abs(output_bits - bits) <= 10);
    }
}

BOOST_AUTO_TEST_CASE(oracle_determinism)
{
    const int bits = 100;
    const unsigned int seed = 42;
    const RandomOracle nostradamus(bits, seed), pythia(bits, seed), paul_the_octopus(bits, seed + 1);

    const size_t iterations = 5;

    for (size_t i = 0; i < iterations; ++i) {
        const auto pythia_output = pythia.next();
        BOOST_CHECK(nostradamus.next() == pythia_output);
        BOOST_CHECK(pythia_output != paul_the_octopus.next());
    }
}

BOOST_AUTO_TEST_CASE(oracle_cache_reset)
{
    const RandomOracle nostradamus(10, 10), pythia(10, 10);

    const mpz_class & nostradamus_output_reference = nostradamus.next();
    const mpz_class nostradamus_output_copy = mpz_class(nostradamus_output_reference);

    const mpz_class pythia_output = mpz_class(pythia.next());

    BOOST_CHECK(nostradamus_output_reference == pythia_output);
    BOOST_CHECK(nostradamus_output_copy == pythia_output);

    RandomOracle::reset_cache();

    BOOST_CHECK(nostradamus_output_reference != pythia_output);
    BOOST_CHECK(nostradamus_output_copy == pythia_output);
}

BOOST_AUTO_TEST_SUITE_END()
