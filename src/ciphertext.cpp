#include "she.hpp"

using std::set;


namespace she
{

CompressedCiphertext::CompressedCiphertext(const ParameterSet & params) : _parameter_set(params)
{
    initialize_oracle();
};

void CompressedCiphertext::initialize_oracle() const
{
    _oracle.reset(new RandomOracle{_parameter_set.ciphertext_size_bits, _parameter_set.oracle_seed});
}

HomomorphicArray CompressedCiphertext::expand() const
{
    _oracle->reset();

    HomomorphicArray result;

    // Restore public element
    const mpz_class & oracle_output = _oracle->next();
    result.set_public_element(oracle_output - _public_element_delta);

    for (const mpz_class & delta : _elements_deltas)
    {
        const mpz_class & oracle_output = _oracle->next();
        result._elements.push_back(oracle_output - delta);
    }

    return result;
}

std::set<mpz_class> HomomorphicArray::public_elements = {};

void HomomorphicArray::set_public_element(const mpz_class & x)
{
    auto result = public_elements.emplace(x);
    _public_element_ptr = result.first;
}

} // namespace she
