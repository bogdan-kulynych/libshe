#include <cmath>

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
using she::sum;
using she::product;


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


BOOST_AUTO_TEST_SUITE(HomomorphicOperationsSuite)

BOOST_AUTO_TEST_CASE(bitwise_xor)
{
    const PrivateKey sk(ParameterSet::generate_parameter_set(22, 5, 42));

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

        {
            const auto c1 = sk.encrypt(input.p1).expand();
            const auto c2 = sk.encrypt(input.p2).expand();

            const auto c1_xor_c2 = c1 ^ c2;

            BOOST_CHECK_EQUAL(c1_xor_c2.degree(), 1);
            BOOST_CHECK(sk.decrypt(c1_xor_c2) == input.p1_xor_p2);
        }

        {
            const auto c1 = sk.encrypt(input.p1).expand();

            const auto c1_xor_p2 = c1 ^ PlaintextArray(input.p2);

            BOOST_CHECK_EQUAL(c1_xor_p2.degree(), 1);
            BOOST_CHECK(sk.decrypt(c1_xor_p2) == input.p1_xor_p2);
        }

        {
            const auto c1 = sk.encrypt(input.p1).expand();

            const auto c1_xor_p2 = PlaintextArray(input.p2) ^ c1;

            BOOST_CHECK_EQUAL(c1_xor_p2.degree(), 1);
            BOOST_CHECK(sk.decrypt(c1_xor_p2) == input.p1_xor_p2);
        }

        {
            const auto c2 = sk.encrypt(input.p2).expand();

            const auto c2_xor_p1 = c2 ^ PlaintextArray(input.p1);

            BOOST_CHECK_EQUAL(c2_xor_p1.degree(), 1);
            BOOST_CHECK(sk.decrypt(c2_xor_p1) == input.p1_xor_p2);
        }

        {
            const auto c2 = sk.encrypt(input.p2).expand();

            const auto c2_xor_p1 = PlaintextArray(input.p1) ^ c2;

            BOOST_CHECK_EQUAL(c2_xor_p1.degree(), 1);
            BOOST_CHECK(sk.decrypt(c2_xor_p1) == input.p1_xor_p2);
        }

        {
            const auto p1_xor_p2 = PlaintextArray(input.p1) ^ PlaintextArray(input.p2);
            BOOST_CHECK(p1_xor_p2.elements() == input.p1_xor_p2);
        }
    }
}

BOOST_AUTO_TEST_CASE(multiple_arrays_sum)
{
    const PrivateKey sk(ParameterSet::generate_parameter_set(22, 5, 42));

    vector<vector<bool> > inputs = {
        {1, 1, 1, 1, 0, 0, 1, 1},
        {0, 0, 0, 1, 0, 1, 0, 1},
        {},
        {1, 1, 1, 1, 0, 0},
        {1, 1, 0, 0, 0, 1, 0, 1},
        {1, 0, 0, 0, 0, 1, 1, 0},
    };

    vector<bool> expected_result =
        {0, 1, 0, 1, 0, 1, 0, 1};

    {
        vector<EncryptedArray> encrypted_inputs;
        for (const auto & input : inputs) {
            encrypted_inputs.push_back(sk.encrypt(input).expand());
        }

        const auto result = sum(encrypted_inputs);
        const auto decrypted_result = sk.decrypt(result);

        BOOST_CHECK_EQUAL(result.degree(), 1);
        BOOST_CHECK(decrypted_result == expected_result);
    }

    {
        vector<PlaintextArray> plaintext_inputs;
        for (const auto & input : inputs) {
            plaintext_inputs.push_back(PlaintextArray(input));
        }

        const auto result = sum(plaintext_inputs);
        BOOST_CHECK(result.elements() == expected_result);
    }
}

BOOST_AUTO_TEST_CASE(bitwise_and)
{
    const PrivateKey sk(ParameterSet::generate_parameter_set(22, 5, 42));

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

        {
            const auto c1 = sk.encrypt(input.p1).expand();
            const auto c2 = sk.encrypt(input.p2).expand();

            const auto c1_and_c2 = c1 & c2;

            BOOST_CHECK_EQUAL(c1_and_c2.degree(), 2);
            BOOST_CHECK(sk.decrypt(c1_and_c2) == input.p1_and_p2);
        }

        {
            const auto c1 = sk.encrypt(input.p1).expand();

            const auto c1_and_p2 = c1 & PlaintextArray(input.p2);

            BOOST_CHECK_EQUAL(c1_and_p2.degree(), 1);
            BOOST_CHECK(sk.decrypt(c1_and_p2) == input.p1_and_p2);
        }

        {
            const auto c1 = sk.encrypt(input.p1).expand();

            const auto c1_and_p2 = PlaintextArray(input.p2) & c1;

            BOOST_CHECK_EQUAL(c1_and_p2.degree(), 1);
            BOOST_CHECK(sk.decrypt(c1_and_p2) == input.p1_and_p2);
        }

        {
            const auto c2 = sk.encrypt(input.p2).expand();

            const auto c2_and_p1 = c2 & PlaintextArray(input.p1);

            BOOST_CHECK_EQUAL(c2_and_p1.degree(), 1);
            BOOST_CHECK(sk.decrypt(c2_and_p1) == input.p1_and_p2);
        }

        {
            const auto c2 = sk.encrypt(input.p2).expand();

            const auto c2_and_p1 = PlaintextArray(input.p1) & c2;

            BOOST_CHECK_EQUAL(c2_and_p1.degree(), 1);
            BOOST_CHECK(sk.decrypt(c2_and_p1) == input.p1_and_p2);
        }

        {
            const auto p1_and_p2 = PlaintextArray(input.p1) & PlaintextArray(input.p2);

            BOOST_CHECK(p1_and_p2.elements() == input.p1_and_p2);
        }
    }
}

BOOST_AUTO_TEST_CASE(multiple_arrays_product)
{
    const PrivateKey sk(ParameterSet::generate_parameter_set(22, 5, 42));

    vector<vector<bool> > inputs = {
        {1, 1, 1, 1, 0, 0, 1, 1},
        {0, 0, 0, 1, 0, 1},
        {1, 1, 1, 1, 0, 0, 0, 1},
        {},
        {1, 1, 0, 1, 0, 1, 0, 1},
        {1, 0, 0, 1, 0, 1, 1, 1},
    };

    vector<bool> expected_result =
        {0, 0, 0, 1, 0, 0, 0, 1};

    {
        vector<EncryptedArray> encrypted_inputs;
        for (const auto & input : inputs) {
            encrypted_inputs.push_back(sk.encrypt(input).expand());
        }

        const auto result = product(encrypted_inputs);
        const auto decrypted_result = sk.decrypt(result);

        BOOST_CHECK_EQUAL(result.degree(), inputs.size());
        BOOST_CHECK(decrypted_result == expected_result);
    }

    {
        vector<PlaintextArray> plaintext_inputs;
        for (const auto & input : inputs) {
            plaintext_inputs.push_back(PlaintextArray(input));
        }

        const auto result = product(plaintext_inputs);
        BOOST_CHECK(result.elements() == expected_result);
    }
}

BOOST_AUTO_TEST_CASE(array_select)
{
    const PrivateKey sk(ParameterSet::generate_parameter_set(22, 4, 42));
    const vector<vector<bool> > raw_plaintext_arrays = {
        {1, 1, 1, 1},
        {0, 1, 0, 1},
        {1, 0, 1, 0},
        {0, 0, 0, 0},
    };

    vector<PlaintextArray> plaintext_arrays;
    for (const auto & raw_plaintext_array : raw_plaintext_arrays) {
        plaintext_arrays.push_back(PlaintextArray(raw_plaintext_array));
    }

    vector<vector<bool> > raw_plaintext_selections = {
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1},
    };

    {
        vector<EncryptedArray> encrypted_selections;
        for (const auto & raw_plaintext_selection : raw_plaintext_selections) {
            encrypted_selections.push_back(sk.encrypt(raw_plaintext_selection).expand());
        }

        for (size_t i = 0; i < encrypted_selections.size(); ++i) {
            const auto result = encrypted_selections[i].select(plaintext_arrays);
            const vector<bool> decrypted_result = sk.decrypt(result);

            BOOST_CHECK_EQUAL(result.degree(), 1);
            BOOST_CHECK(decrypted_result == plaintext_arrays[i].elements());
        }
    }

    {
        vector<PlaintextArray> plaintext_selections;
        for (const auto & raw_plaintext_selection : raw_plaintext_selections) {
            plaintext_selections.push_back(PlaintextArray(raw_plaintext_selection));
        }

        for (size_t i = 0; i < plaintext_selections.size(); ++i) {
            const auto result = plaintext_selections[i].select(plaintext_arrays);
            BOOST_CHECK(result.elements() == plaintext_arrays[i].elements());
        }
    }
}

BOOST_AUTO_TEST_CASE(array_equal)
{
    const PrivateKey sk(ParameterSet::generate_parameter_set(22, 4, 42));
    const vector<vector<bool> > raw_plaintext_arrays = {
        vector<bool>{1, 1, 1, 1},
        vector<bool>{0, 1, 0, 1},
        vector<bool>{1, 0, 1, 0},
        vector<bool>{0, 1, 0, 1},
    };

    vector<PlaintextArray> plaintext_arrays;
    for (const auto & raw_plaintext_array : raw_plaintext_arrays) {
        plaintext_arrays.push_back(PlaintextArray(raw_plaintext_array));
    }

    vector<EncryptedArray> encrypted_arrays;
    for (const auto & raw_plaintext_array : raw_plaintext_arrays) {
        encrypted_arrays.push_back(sk.encrypt(raw_plaintext_array).expand());
    }

    const vector<vector<bool> > raw_plaintext_inputs = {
        vector<bool>{1, 1, 0, 0},
        vector<bool>{1, 1, 1, 1},
        vector<bool>{0, 1, 0, 1},
        vector<bool>{1, 0, 1, 0},
    };

    const vector<vector<bool> > expected_results = {
        {0, 0, 0, 0},
        {1, 0, 0, 0},
        {0, 1, 0, 1},
        {0, 0, 1, 0}
    };

    {
        vector<EncryptedArray> encrypted_inputs;
        for (const auto & raw_plaintext_input : raw_plaintext_inputs) {
            encrypted_inputs.push_back(sk.encrypt(raw_plaintext_input).expand());
        }

        for (size_t i = 0; i < encrypted_inputs.size(); ++i) {
            const auto result = encrypted_inputs[i].equal(encrypted_arrays);
            const vector<bool> decrypted_result = sk.decrypt(result);

            BOOST_CHECK_EQUAL(result.degree(), 4);
            BOOST_CHECK(decrypted_result == expected_results[i]);
        }
    }

    {
        vector<PlaintextArray> plaintext_inputs;
        for (const auto & raw_plaintext_input : raw_plaintext_inputs) {
            plaintext_inputs.push_back(PlaintextArray(raw_plaintext_input));
        }

        for (size_t i = 0; i < plaintext_inputs.size(); ++i) {
            const auto result = plaintext_inputs[i].equal(encrypted_arrays);
            const vector<bool> decrypted_result = sk.decrypt(result);

            BOOST_CHECK_EQUAL(result.degree(), 4);
            BOOST_CHECK(decrypted_result == expected_results[i]);
        }
    }

    {
        vector<EncryptedArray> encrypted_inputs;
        for (const auto & raw_plaintext_input : raw_plaintext_inputs) {
            encrypted_inputs.push_back(sk.encrypt(raw_plaintext_input).expand());
        }

        for (size_t i = 0; i < encrypted_inputs.size(); ++i) {
            const auto result = encrypted_inputs[i].equal(encrypted_arrays);
            const vector<bool> decrypted_result = sk.decrypt(result);

            BOOST_CHECK_EQUAL(result.degree(), 4);
            BOOST_CHECK(decrypted_result == expected_results[i]);
        }
    }

    {
        vector<PlaintextArray> plaintext_inputs;
        for (const auto & raw_plaintext_input : raw_plaintext_inputs) {
            plaintext_inputs.push_back(PlaintextArray(raw_plaintext_input));
        }

        for (size_t i = 0; i < plaintext_inputs.size(); ++i) {
            const auto result = plaintext_inputs[i].equal(plaintext_arrays);
            BOOST_CHECK(result.elements() == expected_results[i]);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()
