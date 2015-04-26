#include <cmath>

#include <boost/test/unit_test.hpp>

#include "she.hpp"
#include "serialization_formats.hpp"

using std::stringstream;
using std::vector;

using she::PrivateKey;
using she::ParameterSet;
using she::CompressedCiphertext;
using she::HomomorphicArray;
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


BOOST_AUTO_TEST_SUITE(HomomorphicArraySuite)

BOOST_AUTO_TEST_CASE(homomorphic_array_construction_accessors_and_comparison)
{
    const vector<bool> plaintext = {1, 0, 1, 0, 1, 1, 1, 1};

    // Construction by encryption
    {
        const PrivateKey sk(ParameterSet::generate_parameter_set(22, 10, 42));

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
        const PrivateKey sk(ParameterSet::generate_parameter_set(22, 10, 42));

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
    const PrivateKey sk(ParameterSet::generate_parameter_set(22, 5, 42));
    const auto array = sk.encrypt({1, 0, 1, 0, 1, 1, 1, 1}).expand();

    HomomorphicArray restored_array;

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

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(HomomorphicOperationsSuite)

BOOST_AUTO_TEST_CASE(homomorphic_array_bitwise_xor)
{
    const PrivateKey sk(ParameterSet::generate_parameter_set(22, 5, 42));

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

            {
                const auto c1 = sk.encrypt(input.p1).expand();
                const auto c2 = sk.encrypt(input.p2).expand();

                const auto c1_xor_c2 = c1 ^ c2;

                BOOST_CHECK_EQUAL(c1_xor_c2.degree(), 1);
                BOOST_CHECK(sk.decrypt(c1_xor_c2) == input.p1_xor_p2);
            }

            {
                const auto c1 = sk.encrypt(input.p1).expand();

                const auto c1_xor_p2 = c1 ^ HomomorphicArray(input.p2);

                BOOST_CHECK_EQUAL(c1_xor_p2.degree(), 1);
                BOOST_CHECK(sk.decrypt(c1_xor_p2) == input.p1_xor_p2);
            }

            {
                const auto c2 = sk.encrypt(input.p2).expand();

                const auto c2_xor_p1 = c2 ^ HomomorphicArray(input.p1);

                BOOST_CHECK_EQUAL(c2_xor_p1.degree(), 1);
                BOOST_CHECK(sk.decrypt(c2_xor_p1) == input.p1_xor_p2);
            }

            {
                const auto p1_xor_p2 = HomomorphicArray(input.p1) ^ HomomorphicArray(input.p2);

                BOOST_CHECK_EQUAL(p1_xor_p2.degree(), 0);
                BOOST_CHECK(sk.decrypt(p1_xor_p2) == input.p1_xor_p2);
            }
        }
    }

    {
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
            vector<HomomorphicArray> encrypted_inputs;
            for (const auto & input : inputs) {
                encrypted_inputs.push_back(sk.encrypt(input).expand());
            }

            const auto result = sum(encrypted_inputs);
            const auto decrypted_result = sk.decrypt(result);

            BOOST_CHECK_EQUAL(result.degree(), 1);
            BOOST_CHECK(decrypted_result == expected_result);
        }

        {
            vector<HomomorphicArray> homomorphic_inputs;
            for (const auto & input : inputs) {
                homomorphic_inputs.push_back(HomomorphicArray(input));
            }

            const auto result = sum(homomorphic_inputs);
            const auto decrypted_result = sk.decrypt(result);

            BOOST_CHECK_EQUAL(result.degree(), 0);
            BOOST_CHECK(decrypted_result == expected_result);
        }
    }
}

BOOST_AUTO_TEST_CASE(homomorphic_array_bitwise_and)
{
    const PrivateKey sk(ParameterSet::generate_parameter_set(22, 5, 42));

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

            {
                const auto c1 = sk.encrypt(input.p1).expand();
                const auto c2 = sk.encrypt(input.p2).expand();

                const auto c1_and_c2 = c1 & c2;

                BOOST_CHECK_EQUAL(c1_and_c2.degree(), 2);
                BOOST_CHECK(sk.decrypt(c1_and_c2) == input.p1_and_p2);
            }

            {
                const auto c1 = sk.encrypt(input.p1).expand();

                const auto c1_and_p2 = c1 & HomomorphicArray(input.p2);

                BOOST_CHECK_EQUAL(c1_and_p2.degree(), 1);
                BOOST_CHECK(sk.decrypt(c1_and_p2) == input.p1_and_p2);
            }

            {
                const auto c2 = sk.encrypt(input.p2).expand();

                const auto c2_and_p1 = c2 & HomomorphicArray(input.p1);

                BOOST_CHECK_EQUAL(c2_and_p1.degree(), 1);
                BOOST_CHECK(sk.decrypt(c2_and_p1) == input.p1_and_p2);
            }

            {
                const auto p1_and_p2 = HomomorphicArray(input.p1) & HomomorphicArray(input.p2);

                BOOST_CHECK_EQUAL(p1_and_p2.degree(), 0);
                BOOST_CHECK(sk.decrypt(p1_and_p2) == input.p1_and_p2);
            }
        }
    }

    {
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
            vector<HomomorphicArray> encrypted_inputs;
            for (const auto & input : inputs) {
                encrypted_inputs.push_back(sk.encrypt(input).expand());
            }

            const auto result = product(encrypted_inputs);
            const auto decrypted_result = sk.decrypt(result);

            BOOST_CHECK_EQUAL(result.degree(), inputs.size());
            BOOST_CHECK(decrypted_result == expected_result);
        }

        {
            vector<HomomorphicArray> homomorphic_inputs;
            for (const auto & input : inputs) {
                homomorphic_inputs.push_back(HomomorphicArray(input));
            }

            const auto result = product(homomorphic_inputs);
            const auto decrypted_result = sk.decrypt(result);

            BOOST_CHECK_EQUAL(result.degree(), 0);
            BOOST_CHECK(decrypted_result == expected_result);
        }
    }
}

BOOST_AUTO_TEST_CASE(homomorphic_array_select)
{
    const PrivateKey sk(ParameterSet::generate_parameter_set(22, 4, 42));
    const vector<vector<bool> > plain_arrays = {
        {1, 1, 1, 1},
        {0, 1, 0, 1},
        {1, 0, 1, 0},
        {0, 0, 0, 0},
    };

    vector<HomomorphicArray> arrays;
    for (const auto & plain_array : plain_arrays) {
        arrays.push_back(HomomorphicArray(plain_array));
    }

    vector<vector<bool> > plain_selections = {
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1},
    };

    {
        vector<HomomorphicArray> encrypted_selections;
        for (const auto & plain_selection : plain_selections) {
            encrypted_selections.push_back(sk.encrypt(plain_selection).expand());
        }

        for (size_t i = 0; i < encrypted_selections.size(); ++i) {
            const auto result = encrypted_selections[i].select(arrays);
            const vector<bool> decrypted_result = sk.decrypt(result);

            BOOST_CHECK_EQUAL(result.degree(), 1);
            BOOST_CHECK(decrypted_result == plain_arrays[i]);
        }
    }

    {
        vector<HomomorphicArray> homomorphic_selections;
        for (const auto & plain_selection : plain_selections) {
            homomorphic_selections.push_back(HomomorphicArray(plain_selection));
        }

        for (size_t i = 0; i < homomorphic_selections.size(); ++i) {
            const auto result = homomorphic_selections[i].select(arrays);
            const vector<bool> decrypted_result = sk.decrypt(result);

            BOOST_CHECK_EQUAL(result.degree(), 1);
            BOOST_CHECK(decrypted_result == plain_arrays[i]);
        }
    }
}

BOOST_AUTO_TEST_CASE(homomorphic_array_equal)
{
    const PrivateKey sk(ParameterSet::generate_parameter_set(22, 4, 42));
    const vector<vector<bool> > plain_arrays = {
        vector<bool>{1, 1, 1, 1},
        vector<bool>{0, 1, 0, 1},
        vector<bool>{1, 0, 1, 0},
        vector<bool>{0, 1, 0, 1},
    };

    vector<HomomorphicArray> arrays;
    for (const auto & plain_array : plain_arrays) {
        arrays.push_back(HomomorphicArray(plain_array));
    }

    const vector<vector<bool> > plain_inputs = {
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
        vector<HomomorphicArray> encrypted_inputs;
        for (const auto & plain_input : plain_inputs) {
            encrypted_inputs.push_back(sk.encrypt(plain_input).expand());
        }

        for (size_t i = 0; i < encrypted_inputs.size(); ++i) {
            const auto result = encrypted_inputs[i].equal(arrays);
            const vector<bool> decrypted_result = sk.decrypt(result);
            BOOST_CHECK(decrypted_result == expected_results[i]);
        }
    }

    {
        vector<HomomorphicArray> homomorphic_inputs;
        for (const auto & plain_input : plain_inputs) {
            homomorphic_inputs.push_back(HomomorphicArray(plain_input));
        }

        for (size_t i = 0; i < homomorphic_inputs.size(); ++i) {
            const auto result = homomorphic_inputs[i].equal(arrays);
            const vector<bool> decrypted_result = sk.decrypt(result);
            BOOST_CHECK(decrypted_result == expected_results[i]);
        }
    }
}

BOOST_AUTO_TEST_CASE(homomorphic_arrays_concat)
{
    const PrivateKey sk(ParameterSet::generate_parameter_set(22, 4, 42));

    {
        HomomorphicArray array;
        const auto homomorphic_plaintext = HomomorphicArray({1, 1, 1, 1});
        array.extend(homomorphic_plaintext);

        BOOST_CHECK(array.elements() == homomorphic_plaintext.elements());
    }

    {
        const auto homomorphic_result = HomomorphicArray({1, 1, 0, 0, 0, 0, 1, 1});

        HomomorphicArray array({1, 1, 0, 0});
        const auto homomorphic_plaintext = HomomorphicArray({0, 0, 1, 1});
        array.extend(homomorphic_plaintext);

        BOOST_CHECK(array.elements() == homomorphic_result.elements());
    }

    {
        const vector< vector<bool>> inputs = {
            {0, 1, 0, 1},
            {1, 0, 1, 0},
            {0, 0, 0, 0},
            {1, 1, 1, 1}
        };

        vector<HomomorphicArray> homomorphic_inputs;
        for (const auto & input : inputs) {
            homomorphic_inputs.push_back(HomomorphicArray(input));
        }

        const auto homomorphic_result = HomomorphicArray({0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1});

        BOOST_CHECK(concat(homomorphic_inputs) == homomorphic_result);
    }
}

BOOST_AUTO_TEST_SUITE_END()
