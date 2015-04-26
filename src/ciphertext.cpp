#include <cmath>

#include "she.hpp"
#include "exceptions.hpp"

using std::min;
using std::set;
using std::vector;


namespace she
{

CompressedCiphertext::CompressedCiphertext(const ParameterSet & params) noexcept :
  _parameter_set(params)
{
    initialize_oracle();
}

void CompressedCiphertext::initialize_oracle() const noexcept
{
    _oracle.reset(new RandomOracle{_parameter_set.ciphertext_size_bits, _parameter_set.oracle_seed});
}

bool CompressedCiphertext::operator==(const CompressedCiphertext & other) const noexcept
{
    return (_parameter_set == other._parameter_set)
        && (_public_element_delta == other._public_element_delta)
        && (_elements_deltas == other._elements_deltas);
}

HomomorphicArray CompressedCiphertext::expand() const noexcept
{
    _oracle->reset();

    // Restore public element
    const auto & oracle_output = _oracle->next();
    const auto public_element = oracle_output - _public_element_delta;

    HomomorphicArray result(public_element, _parameter_set.degree());

    // Restore ciphertext elements
    for (const auto & delta : _elements_deltas) {
        const auto & oracle_output = _oracle->next();
        result._elements.push_back(oracle_output - delta);
    }

    return result;
}


std::set<mpz_class> HomomorphicArray::public_elements = {};

HomomorphicArray::HomomorphicArray(vector<bool> plaintext) noexcept :
  _degree(0),
  _max_degree(0)
{
    for(const bool bit : plaintext) {
        _elements.push_back(bit);
    }
    set_public_element(2);
}

HomomorphicArray::HomomorphicArray(const mpz_class & x, unsigned int max_degree) noexcept :
  _degree(0),
  _max_degree(max_degree)
{
    set_public_element(x);
}

bool HomomorphicArray::operator==(const HomomorphicArray & other) const noexcept
{
    return (_elements == other._elements)
        && (*_public_element_ptr == *other._public_element_ptr);
}

void HomomorphicArray::set_public_element(const mpz_class & x) noexcept
{
    auto result = public_elements.emplace(x);
    _public_element_ptr = result.first;
}

HomomorphicArray & HomomorphicArray::operator^=(const HomomorphicArray & other)
{
    const auto n = min(_elements.size(), other._elements.size());

    for (size_t i = 0; i < n; ++i) {
        _elements[i] += other._elements[i];
        _elements[i] %= *_public_element_ptr;
    }

    for (size_t i = n; i < other._elements.size(); ++i) {
        _elements.push_back(other._elements[i]);
    }

    return *this;
}

HomomorphicArray & HomomorphicArray::operator&=(const HomomorphicArray & other)
{
    const auto n = min(_elements.size(), other._elements.size());

    for (size_t i = 0; i < n; ++i) {
        _elements[i] *= other._elements[i];
        _elements[i] %= *_public_element_ptr;
    }

    for (size_t i = n; i < other._elements.size(); ++i) {
        _elements.push_back(other._elements[i]);
    }

    return *this;
}

const HomomorphicArray HomomorphicArray::equal(const std::vector<HomomorphicArray> & arrays) const
{
    HomomorphicArray result(*_public_element_ptr, _max_degree);

    for (const auto & array : arrays) {
        auto difference = *this ^ array;

        mpz_class all = 1;
        for (const auto & element : difference._elements)
        {
            all *= (element + 1) % *_public_element_ptr;
        }

        result._elements.push_back(all);
    }

    return result;
}

const HomomorphicArray HomomorphicArray::select(const std::vector<HomomorphicArray> & arrays) const
{
    HomomorphicArray result(*_public_element_ptr, _max_degree);

    for (size_t i = 0; i < min(_elements.size(), arrays.size()); ++i) {

        HomomorphicArray picked(*_public_element_ptr, _max_degree);

        for (const auto & element : arrays[i]._elements) {
            picked._elements.push_back(element * _elements[i] % *_public_element_ptr);
        }

        result ^= picked;
    }

    return result;
}

HomomorphicArray & HomomorphicArray::extend(const HomomorphicArray & other)
{
    for (const auto & element : other._elements) {
        _elements.push_back(element);
    }

    return *this;
}

HomomorphicArray sum(const vector<HomomorphicArray> & arrays)
{
    ASSERT(arrays.size() > 0, "Empty input");

    HomomorphicArray result(arrays[0].public_element(), arrays[0].max_degree());

    for (const auto & array : arrays) {
        result ^= array;
    }

    return result;
}

HomomorphicArray product(const vector<HomomorphicArray> & arrays)
{
    ASSERT(arrays.size() > 0, "Empty input");

    HomomorphicArray result(arrays[0].public_element(), arrays[0].max_degree());

    for (const auto & array : arrays) {
        result &= array;
    }

    return result;
}

HomomorphicArray concat(const vector<HomomorphicArray> & arrays)
{
    ASSERT(arrays.size() > 0, "Empty input");

    HomomorphicArray result(arrays[0].public_element(), arrays[0].max_degree());

    for (const auto & array : arrays) {
        result.extend(array);
    }

    return result;
}

} // namespace she
