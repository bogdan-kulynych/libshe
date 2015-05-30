#include <ctime>
#include <cstdlib>
#include <iostream>
#include <random>
#include <memory>

#include "she.hpp"
#include "utils.hpp"

using std::cout;
using std::endl;
using std::boolalpha;
using std::vector;
using std::unique_ptr;
using std::move;

using she::PrivateKey;
using she::ParameterSet;
using she::CompressedCiphertext;
using she::EncryptedArray;
using she::PlaintextArray;


vector<bool> dec_to_bits(unsigned int num, unsigned int bit_size)
{
    vector<bool> result;
    for (int i = bit_size - 1; i >= 0; --i) {
        result.push_back((num >> i) & 1);
    }
    return result;
}

vector<bool> random_bits(unsigned int bit_size)
{
    vector<bool> result {};
    for (unsigned int i = 0; i < bit_size; ++i) {
        bool random_bit = rand() % 2;
        result.push_back(random_bit);
    }

    return result;
}

void
randomly_populate_database( vector<PlaintextArray> * database
                          , unsigned int database_size
                          , unsigned int record_size)
{
    for (unsigned int i = 0; i < database_size; ++i) {
        database->push_back(PlaintextArray(random_bits(record_size)));
    }
}

void build_database_indexes( vector<PlaintextArray> * database_indexes
                         , unsigned int database_size
                         , unsigned int index_size)
{
    for (unsigned int i = 0; i < database_size; ++i) {
        database_indexes->push_back(dec_to_bits(i, index_size));
    }
}

PrivateKey generate_key( unsigned int security
                       , unsigned int record_size)
{
    START_TIMER("KEY GENERATION", "CLIENT");

    const auto params = ParameterSet::generate_parameter_set(security, record_size, 42);
    auto result = PrivateKey(params);

    END_TIMER();

    return result;
}

CompressedCiphertext
generate_query( const PrivateKey & sk
              , const vector<bool> & index_bits)
{
    START_TIMER("QUERY GENERATION", "CLIENT");

    auto result = sk.encrypt(index_bits);

    END_TIMER();
    return result;
}

EncryptedArray expand_ciphertext(const CompressedCiphertext & ctxt)
{
    START_TIMER("QUERY CIPHERTEXT EXPANSION", "SERVER");

    auto result = ctxt.expand();

    END_TIMER();
    return result;
}

EncryptedArray
calculate_selection_vector( const EncryptedArray & query
                          , const vector<PlaintextArray> & database_indexes)
{
    START_TIMER("SELECTION VECTOR HOMOMORPHIC CALCULATION", "SERVER");

    auto result = query.equal(database_indexes);

    END_TIMER();
    return result;
}

EncryptedArray
calculate_response( const EncryptedArray & sv
                  , const vector<PlaintextArray> & database)
{
    START_TIMER("RESPONSE HOMOMORPHIC CALCULATION", "SERVER");

    auto result = sv.select(database);

    END_TIMER();
    return result;
}

PlaintextArray
decrypt_response( const PrivateKey & sk
                  , const EncryptedArray & response)
{
    START_TIMER("RESPONSE DECRYPTION", "CLIENT");

    auto result = sk.decrypt(response);

    END_TIMER();
    return result;
}


int main()
{
    srand (time(NULL));

    // Security level. 62 for 62-bit security
    unsigned int security = 62;

    // Number of records in a database
    unsigned int database_size = 16;

    // Size of every record in bits (note that in general sizes can vary)
    unsigned int record_size = 64;

    // Size of database index in bits.
    // To avoid problems should be equal to floor(log2(database_size) + 1
    unsigned int index_size = 4;

    cout << "Security:      " << security << endl;
    cout << "Database size: " << database_size << endl;
    cout << "Record size:   " << record_size << endl;
    cout << "Index size:    " << index_size << endl << endl;

    // Preparation: Generate random database
    vector<PlaintextArray> database;
    randomly_populate_database(&database, database_size, record_size);

    // Preparation: Generate database indexes
    vector<PlaintextArray> database_indexes;
    build_database_indexes(&database_indexes, database_size, index_size);

    // Generate key
    const auto sk = move(generate_key(security, index_size));

    // Pick random index and generate compressed query ciphertext
    // This is the slowest part. Should consider options to move it to client side.
    const auto index = rand() % database_size;
    const auto compressed_ciphertext = move(generate_query(sk, dec_to_bits(index, index_size)));

    // Expand the compressed ciphertext
    const auto encrypted_query = move(expand_ciphertext(compressed_ciphertext));

    // Calculate homomorphic selection vector
    const auto selector = move(calculate_selection_vector(encrypted_query, database_indexes));

    // Homomorphically calculate response
    const auto encrypted_response = move(calculate_response(selector, database));

    // Decrypt response
    std::vector<bool> response = move(decrypt_response(sk, encrypted_response));

    TIMER_STATS();

    // Show response if it is small enough
    if (record_size < 80) {
        cout << "Queried database index:" << endl;
        for (const auto & bit : database[index].elements()) {
            cout << bit;
        }
        cout << endl;

        cout << "Obtained result:" << endl;
        for (const auto & bit : response) {
            cout << bit;
        }
        cout << endl;
    }

    cout << "Correct result? " << boolalpha << (database[index] == response) << endl;

    return 0;
}