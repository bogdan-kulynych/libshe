#include <cmath>
#include <iostream>

#include <boost/test/unit_test.hpp>

#include "she.hpp"
#include "serialization_formats.hpp"

using std::stringstream;
using std::vector;

using she::PrivateKey;
using she::ParameterSet;
using she::CompressedCiphertext;
using she::HomomorphicArray;


BOOST_AUTO_TEST_SUITE(CompressedCiphertextSuite)

BOOST_AUTO_TEST_CASE(compressed_ciphertext_construction_accessors_and_comparison)
{
    const PrivateKey sk(ParameterSet::generate_parameter_set(62, 10, 42));
    vector<bool> plaintext = {1, 0, 1, 0, 1, 1, 1, 1};

    {
        const auto compressed_ciphertext = sk.encrypt(plaintext);

        BOOST_CHECK_EQUAL(compressed_ciphertext.size(), plaintext.size());
        BOOST_CHECK_EQUAL(compressed_ciphertext.elements_deltas().size(), plaintext.size());
        BOOST_CHECK(compressed_ciphertext.public_element_delta() < sk.private_element());
    }

    {
        // These should be different, because new noises are generated on every encryption
        const auto c1 = sk.encrypt(plaintext);
        const auto c2 = sk.encrypt(plaintext);
        BOOST_CHECK(c1 != c2);
        BOOST_CHECK(!(c1 == c2));
    }
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

BOOST_AUTO_TEST_CASE_TEMPLATE(compressed_ciphertext_serialization, Format, Formats)
{
    const PrivateKey sk(ParameterSet::generate_parameter_set(62, 10, 42));
    const auto ciphertext = sk.encrypt({1, 0, 1, 0, 1, 1, 1, 1});

    CompressedCiphertext restored_ciphertext;

    stringstream ss;
    {
        typename Format::oarchive oa(ss);
        oa << BOOST_SERIALIZATION_NVP(ciphertext);
    }
    {
        typename Format::iarchive ia(ss);
        ia >> BOOST_SERIALIZATION_NVP(restored_ciphertext);
    }

    BOOST_CHECK(ciphertext == restored_ciphertext);
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(HomomorphicArraySuite)

BOOST_AUTO_TEST_CASE(homomorphic_array_construction_accessors_and_comparison)
{
    const vector<bool> plaintext = {1, 0, 1, 0, 1, 1, 1, 1};

    // Construction by encryption
    {
        const PrivateKey sk(ParameterSet::generate_parameter_set(62, 10, 42));

        const auto compressed_ciphertext = sk.encrypt(plaintext);
        const auto a1 = compressed_ciphertext.expand();

        BOOST_CHECK_EQUAL(a1.size(), plaintext.size());
        BOOST_CHECK_EQUAL(a1.elements().size(), plaintext.size());
        BOOST_CHECK_EQUAL(a1.max_degree(), sk.parameter_set().degree());
        BOOST_CHECK(a1.public_element() % sk.private_element() == 0);

        const auto a2 = compressed_ciphertext.expand();
        BOOST_CHECK(a1 == a2);
        BOOST_CHECK(!(a1 != a2));
    }

    // More comparison tests
    {
        const PrivateKey sk(ParameterSet::generate_parameter_set(62, 10, 42));

        const auto c1 = sk.encrypt(plaintext);
        const auto c2 = sk.encrypt(plaintext);
        const auto a1 = c1.expand();
        const auto a2 = c2.expand();

        BOOST_CHECK_EQUAL(a1.size(), a2.size());
        BOOST_CHECK_EQUAL(a1.elements().size(), a2.elements().size());
        BOOST_CHECK_EQUAL(a1.max_degree(), a2.max_degree());
        BOOST_CHECK(a1.public_element() % sk.private_element() == 0);
        BOOST_CHECK(a2.public_element() % sk.private_element() == 0);

        BOOST_CHECK(a1 != a2);
        BOOST_CHECK(!(a1 == a2));
    }

    // Construction from plaintext
    {
        const HomomorphicArray a1(plaintext);

        BOOST_CHECK_EQUAL(a1.size(), plaintext.size());
        BOOST_CHECK(a1.public_element() == 2);

        const HomomorphicArray a2(plaintext);
        BOOST_CHECK(a1 == a2);
        BOOST_CHECK(!(a1 != a2));
    }

    // Empty construction with public element
    {
        const mpz_class x = 42;
        const HomomorphicArray a1(x, 10);

        BOOST_CHECK_EQUAL(a1.size(), 0);
        BOOST_CHECK_EQUAL(a1.elements().size(), 0);
        BOOST_CHECK(a1.public_element() == x);

        const HomomorphicArray a2(x, 10);
        BOOST_CHECK(a1 == a2);
        BOOST_CHECK(!(a1 != a2));
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(homomorphic_array_serialization, Format, Formats)
{
    const PrivateKey sk(ParameterSet::generate_parameter_set(20, 5, 42));
    const auto array = sk.encrypt({1, 0, 1, 0, 1, 1, 1, 1}).expand();

    HomomorphicArray restored_array;

    stringstream ss;
    {
        typename Format::oarchive oa(ss);
        oa << array;
    }
    {
        typename Format::iarchive ia(ss);
        ia >> restored_array;
    }

    BOOST_CHECK(array == restored_array);
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(HomomorphicOperationsSuite)

BOOST_AUTO_TEST_CASE(bitwise_xor)
{
    struct Input {
        vector<bool> p1;
        vector<bool> p2;
        vector<bool> p1_xor_p2;
    };

    const auto inputs = {
        Input{
            {1, 0, 1, 0, 1, 1, 1, 1},
            {1, 0, 1, 0, 1, 1, 1, 1},
            {0, 0, 0, 0, 0, 0, 0, 0}
        },

        Input{
            {1, 0, 1, 0, 1, 0, 1, 0},
            {1, 0, 1, 0, 1, 1, 1, 1},
            {0, 0, 0, 0, 0, 1, 0, 1}
        },

        Input{
            {1, 0, 1, 0, 1, 1, 1, 1},
            {0, 1, 0, 1, 0, 0, 0, 0},
            {1, 1, 1, 1, 1, 1, 1, 1}
        },
    };

    for (const auto & input : inputs) {

        const PrivateKey sk(ParameterSet::generate_parameter_set(20, 5, 42));

        {
            const auto c1 = sk.encrypt(input.p1).expand();
            const auto c2 = sk.encrypt(input.p2).expand();

            const auto c1_xor_c2 = c1 ^ c2;
            BOOST_CHECK(sk.decrypt(c1_xor_c2) == input.p1_xor_p2);
        }

        {
            const auto c1 = sk.encrypt(input.p1).expand();

            const auto c1_xor_p2 = c1 ^ HomomorphicArray(input.p2);
            BOOST_CHECK(sk.decrypt(c1_xor_p2) == input.p1_xor_p2);
        }

        {
            const auto c1 = sk.encrypt(input.p1).expand();

            const auto c1_xor_p2 = c1 ^ HomomorphicArray(input.p2);
            BOOST_CHECK(sk.decrypt(c1_xor_p2) == input.p1_xor_p2);
        }
    }
}

BOOST_AUTO_TEST_CASE(bitwise_and)
{
    struct Input {
        vector<bool> p1;
        vector<bool> p2;
        vector<bool> p1_and_p2;
    };

    const auto inputs = {
        Input{
            {1, 0, 1, 0, 1, 1, 1, 1},
            {1, 0, 1, 0, 1, 1, 1, 1},
            {1, 0, 1, 0, 1, 1, 1, 1}
        },

        Input{
            {1, 0, 1, 0, 1, 0, 1, 0},
            {1, 0, 1, 0, 1, 1, 1, 1},
            {1, 0, 1, 0, 1, 0, 1, 0}
        },

        Input{
            {1, 0, 1, 0, 1, 1, 1, 1},
            {0, 1, 0, 1, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0}
        },
    };

    for (const auto & input : inputs) {

        const PrivateKey sk(ParameterSet::generate_parameter_set(20, 5, 42));

        {
            const auto c1 = sk.encrypt(input.p1).expand();
            const auto c2 = sk.encrypt(input.p2).expand();

            const auto c1_and_c2 = c1 & c2;
            BOOST_CHECK(sk.decrypt(c1_and_c2) == input.p1_and_p2);
        }

        {
            const auto c1 = sk.encrypt(input.p1).expand();

            const auto c1_and_p2 = c1 & HomomorphicArray(input.p2);
            BOOST_CHECK(sk.decrypt(c1_and_p2) == input.p1_and_p2);
        }

        {
            const auto c1 = sk.encrypt(input.p1).expand();

            const auto c1_and_p2 = c1 & HomomorphicArray(input.p2);
            BOOST_CHECK(sk.decrypt(c1_and_p2) == input.p1_and_p2);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()
