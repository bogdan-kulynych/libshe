#include <cmath>

#include <boost/test/unit_test.hpp>

#include "she.hpp"
#include "serialization_formats.hpp"

using std::vector;

using she::PrivateKey;
using she::ParameterSet;
using she::CompressedCiphertext;
using she::PlaintextArray;
using she::EncryptedArray;

using she::sum;
using she::product;


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
