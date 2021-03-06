#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE CiphertextModule
#include <cstddef>
#include <boost/test/unit_test.hpp>

#include "she.hpp"
#include "serialization_formats.hpp"

using std::stringstream;
using std::vector;

using she::PrivateKey;
using she::ParameterSet;
using she::CompressedCiphertext;
using she::PlaintextArray;
using she::EncryptedArray;


BOOST_AUTO_TEST_SUITE(CompressedCiphertextSuite)

BOOST_AUTO_TEST_CASE(compressed_ciphertext_construction_accessors_and_comparison)
{
    const PrivateKey sk(ParameterSet::generate_parameter_set(42, 10, 42));
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
    const PrivateKey sk(ParameterSet::generate_parameter_set(42, 5, 42));
    const vector<bool> plaintext = {1, 0, 1, 0, 1, 1, 1, 1};
    const auto compressed_ciphertext = sk.encrypt(plaintext);
    const auto expanded_ciphertext = compressed_ciphertext.expand();
    const auto restored_plaintext = sk.decrypt(expanded_ciphertext);

    BOOST_CHECK(expanded_ciphertext.public_element() % sk.private_element() == 0);
    BOOST_CHECK_EQUAL(restored_plaintext.size(), plaintext.size());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(compressed_ciphertext_serialization, Format, Formats)
{
    const PrivateKey sk(ParameterSet::generate_parameter_set(42, 10, 42));
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


BOOST_AUTO_TEST_SUITE(EncryptedArraySuite)

BOOST_AUTO_TEST_CASE(encrypted_array_construction_from_encryption)
{
    const vector<bool> plaintext = {1, 0, 1, 0, 1, 1, 1, 1};

    const PrivateKey sk(ParameterSet::generate_parameter_set(22, 10, 42));

    const auto compressed_ciphertext = sk.encrypt(plaintext);
    const auto a1 = compressed_ciphertext.expand();

    BOOST_CHECK_EQUAL(a1.size(), plaintext.size());
    BOOST_CHECK_EQUAL(a1.elements().size(), plaintext.size());
    BOOST_CHECK_EQUAL(a1.degree(), 1);
    BOOST_CHECK_EQUAL(a1.max_degree(), sk.parameter_set().degree());
    BOOST_CHECK(a1.public_element() % sk.private_element() == 0);
}

BOOST_AUTO_TEST_CASE(encrypted_array_comparison)
{
    const vector<bool> plaintext = {1, 0, 1, 0, 1, 1, 1, 1};

    const PrivateKey sk(ParameterSet::generate_parameter_set(22, 10, 42));

    const auto c1 = sk.encrypt(plaintext);
    const auto c2 = sk.encrypt(plaintext);
    const auto a1 = c1.expand();
    const auto a2 = c2.expand();

    BOOST_CHECK_EQUAL(a1.size(), a2.size());
    BOOST_CHECK_EQUAL(a1.elements().size(), a2.elements().size());
    BOOST_CHECK_EQUAL(a1.degree(), a2.degree());
    BOOST_CHECK_EQUAL(a1.max_degree(), a2.max_degree());
    BOOST_CHECK(a1.public_element() % sk.private_element() == 0);
    BOOST_CHECK(a2.public_element() % sk.private_element() == 0);

    BOOST_CHECK(a1 != a2);
    BOOST_CHECK(!(a1 == a2));
}

BOOST_AUTO_TEST_CASE(encrypted_array_empty_construction)
{
    const mpz_class x = 42;
    const EncryptedArray a1(x, 10);

    BOOST_CHECK_EQUAL(a1.size(), 0);
    BOOST_CHECK_EQUAL(a1.elements().size(), 0);
    BOOST_CHECK_EQUAL(a1.degree(), 1);
    BOOST_CHECK(a1.public_element() == x);

    const EncryptedArray a2(x, 10);
    BOOST_CHECK(a1 == a2);
    BOOST_CHECK(!(a1 != a2));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(encrypted_array_serialization, Format, Formats)
{
    const PrivateKey sk(ParameterSet::generate_parameter_set(22, 5, 42));
    const auto array = sk.encrypt({1, 0, 1, 0, 1, 1, 1, 1}).expand();

    EncryptedArray restored_array;

    stringstream ss;
    {
        typename Format::oarchive oa(ss);
        oa << BOOST_SERIALIZATION_NVP(array);
    }
    {
        typename Format::iarchive ia(ss);
        ia >> BOOST_SERIALIZATION_NVP(restored_array);
    }

    BOOST_CHECK(array == restored_array);
}

BOOST_AUTO_TEST_CASE(encrypted_arrays_extend_empty)
{
    const PrivateKey sk(ParameterSet::generate_parameter_set(22, 5, 42));

    auto array = sk.encrypt({}).expand();
    const auto plaintext = PlaintextArray({1, 1, 1, 1});
    const auto ciphertext = sk.encrypt(plaintext).expand();
    array.extend(ciphertext);

    BOOST_CHECK(sk.decrypt(array) == plaintext.elements());
}

BOOST_AUTO_TEST_CASE(encrypted_arrays_concatenation_empty)
{
    const auto expected_result = PlaintextArray({1, 1, 0, 0, 0, 0, 1, 1});

    const PrivateKey sk(ParameterSet::generate_parameter_set(22, 10, 42));

    auto array = sk.encrypt({1, 1, 0, 0}).expand();
    const auto ciphertext = sk.encrypt({0, 0, 1, 1}).expand();
    array.extend(ciphertext);

    BOOST_CHECK(sk.decrypt(array) == expected_result.elements());
}

BOOST_AUTO_TEST_CASE(encrypted_arrays_concat)
{
    const PrivateKey sk(ParameterSet::generate_parameter_set(22, 5, 42));

    const vector< vector<bool>> raw_plaintext_inputs = {
        {0, 1, 0, 1},
        {1, 0, 1, 0},
        {0, 0, 0, 0},
        {1, 1, 1, 1}
    };

    vector<EncryptedArray> encrypted_inputs;
    for (const auto & raw_plaintext_input : raw_plaintext_inputs) {
        encrypted_inputs.push_back(sk.encrypt(raw_plaintext_input).expand());
    }

    const auto expected_result = PlaintextArray({0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1});
    auto concatenated = concat(encrypted_inputs);

    BOOST_CHECK(sk.decrypt(concatenated) == expected_result.elements());
}

BOOST_AUTO_TEST_SUITE_END()