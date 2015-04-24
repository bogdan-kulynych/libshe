#include <boost/test/unit_test.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <sstream>
#include <vector>

#include "she.hpp"

using std::vector;
using std::stringstream;

using boost::archive::text_oarchive;
using boost::archive::text_iarchive;

using she::ParameterSet;
using she::PrivateKey;
using she::CompressedCiphertext;
using she::HomomorphicArray;


BOOST_AUTO_TEST_SUITE(WorkflowSuite)

BOOST_AUTO_TEST_CASE(remote_execution_simulation)
{
    stringstream ss1;
    stringstream ss2;

    PrivateKey * sk;

    // ----------- CLIENT ----------------------------------------------
    {
        const auto params = ParameterSet::generate_parameter_set(62, 1, 42);

        // Generate private key
        sk = new PrivateKey(params);

        // Encrypt plaintext
        const vector<bool> plaintext = {1, 0, 1, 0, 1, 0, 1, 0};
        const auto compressed_ciphertext = sk->encrypt(plaintext);

        text_oarchive oa1(ss1);
        oa1 << compressed_ciphertext;
    }

    // ----------- SERVER ----------------------------------------------

    {
        CompressedCiphertext received_compressed_ciphertext;
        text_iarchive ia1(ss1);
        ia1 >> received_compressed_ciphertext;

        // Expand the ciphertext to perform operations
        const HomomorphicArray ciphertext = received_compressed_ciphertext.expand();

        // Execute some algorithm
        const vector<bool> another_plaintext = { 1, 1, 1, 1, 1, 1, 1, 1 };
        const auto response = ciphertext ^ HomomorphicArray(another_plaintext);

        // Serialize result
        text_oarchive oa2(ss2);
        oa2 << response;
    }

    // ----------- CLIENT ----------------------------------------------

    {
        HomomorphicArray received_response;
        text_iarchive ia2(ss2);
        ia2 >> received_response;

        const auto decrypted_response = sk->decrypt(received_response);
        const vector<bool> expected_result = {0, 1, 0, 1, 0, 1, 0, 1};

        BOOST_CHECK(decrypted_response == expected_result);
    }
}

BOOST_AUTO_TEST_SUITE_END()
