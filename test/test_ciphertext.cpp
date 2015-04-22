#include <cmath>
#include <iostream>

#include <boost/test/unit_test.hpp>

#include "she.hpp"
#include "serialization_formats.hpp"

using std::stringstream;
using std::vector;

using she::CompressedCiphertext;
using she::PrivateKey;
using she::ParameterSet;


BOOST_AUTO_TEST_SUITE(CompressedCiphertextSuite)

BOOST_AUTO_TEST_CASE(compressed_ciphertext_construction)
{
    const PrivateKey sk(ParameterSet::generate_parameter_set(62, 10, 42));
    const auto compressed_ciphertext = sk.encrypt({1, 0, 1, 0, 1, 1, 1, 1});
}

BOOST_AUTO_TEST_CASE(compressed_ciphertext_correct_expansion)
{
    const PrivateKey sk(ParameterSet::generate_parameter_set(62, 5, 42));
    const vector<bool> plaintext = {1, 0, 1, 0, 1, 1, 1, 1};
    const auto compressed_ciphertext = sk.encrypt(plaintext);
    const auto expanded_ciphertext = compressed_ciphertext.expand();
    const auto restored_plaintext = sk.decrypt(expanded_ciphertext);

    BOOST_CHECK(expanded_ciphertext.public_element() % sk.private_element() == 0);
    BOOST_CHECK_EQUAL(restored_plaintext.size(), plaintext.size());
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(HomomorphicArraySuite)

BOOST_AUTO_TEST_CASE(homomorphic_array_construction)
{
    const PrivateKey sk(ParameterSet::generate_parameter_set(62, 10, 42));
    const auto compressed_ciphertext = sk.encrypt({1, 0, 1, 0, 1, 1, 1, 1});
    const auto expanded_ciphertext = compressed_ciphertext.expand();
}

BOOST_AUTO_TEST_CASE(homomorphic_array_construction_from_plaintext)
{
    const vector<bool> plaintext = {1, 0, 1, 0, 1, 1, 1, 1};
    // const auto homomorphic_array = homomorphic_array(plaintext);
}

BOOST_AUTO_TEST_SUITE_END()
