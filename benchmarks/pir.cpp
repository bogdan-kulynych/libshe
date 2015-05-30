#include <ctime>
#include <cstdlib>
#include <iostream>
#include <random>
#include <memory>

#include "she.hpp"
#include "utils.hpp"

using std::cout;
using std::endl;
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

void build_database_index( vector<PlaintextArray> * database_index
                         , unsigned int database_size
                         , unsigned int index_size)
{
    for (unsigned int i = 0; i < database_size; ++i) {
        database_index->push_back(dec_to_bits(i, index_size));
    }
}

PrivateKey generate_key( unsigned int security
                       , unsigned int record_size)
{
    START_TIMER("KEY GENERATION");

    const auto params = ParameterSet::generate_parameter_set(security, record_size, 42);
    auto result = PrivateKey(params);

    END_TIMER();

    return result;
}

CompressedCiphertext
generate_query( const PrivateKey & sk
              , const vector<bool> & index_bits)
{
    START_TIMER("QUERY GENERATION");

    auto result = sk.encrypt(index_bits);

    END_TIMER();
    return result;
}

EncryptedArray expand_ciphertext(const CompressedCiphertext & ctxt)
{
    START_TIMER("QUERY CIPHERTEXT EXPANSION");

    auto result = ctxt.expand();

    END_TIMER();
    return result;
}

EncryptedArray
calculate_selection_vector( const EncryptedArray & query
                          , const vector<PlaintextArray> & database_index)
{
    START_TIMER("SELECTION VECTOR HOMOMORPHIC CALCULATION");

    auto result = query.equal(database_index);

    END_TIMER();
    return result;
}

EncryptedArray
calculate_response( const EncryptedArray & sv
                  , const vector<PlaintextArray> & database)
{
    START_TIMER("RESPONSE HOMOMORPHIC CALCULATION");

    auto result = sv.select(database);

    END_TIMER();
    return result;
}

PlaintextArray
decrypt_response( const PrivateKey & sk
                  , const EncryptedArray & response)
{
    START_TIMER("RESPONSE DECRYPTION");

    auto result = sk.decrypt(response);

    END_TIMER();
    return result;
}


int main()
{
    srand (time(NULL));

    unsigned int security = 42;
    unsigned int database_size = 8;
    unsigned int record_size = 10;
    unsigned int index_size = 3;

    // Preparation: Generate random database
    vector<PlaintextArray> database;
    randomly_populate_database(&database, database_size, record_size);

    // Preparation: Generate database index
    vector<PlaintextArray> database_index;
    build_database_index(&database_index, database_size, index_size);

    // Generate key
    auto const sk = move(generate_key(security, record_size));

    // Pick random index and generate compressed query ciphertext
    auto const ctxt = move(generate_query(sk, random_bits(index_size)));

    // Expand the compressed ciphertext
    auto const query = move(expand_ciphertext(ctxt));

    // Calculate homomorphic selection vector
    auto const sv = move(calculate_selection_vector(query, database_index));

    // Homomorphically calculate response
    auto const er = move(calculate_response(sv, database));

    // Decrypt response
    auto const pr = move(decrypt_response(sk, er));

    return 0;
}