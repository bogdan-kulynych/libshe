#include <cmath>

#include "she.hpp"
#include "she/exceptions.hpp"

using std::min;
using std::max;
using std::set;
using std::vector;


namespace she
{

std::set<mpz_class> EncryptedArray::public_elements = {};

EncryptedArray::EncryptedArray(const mpz_class & x, unsigned int max_degree, unsigned int degree) noexcept :
  _degree(degree),
  _max_degree(max_degree)
{
    // ASSERT(degree >= 1, "Degree must be at least 1");

    set_public_element(x);
}

bool EncryptedArray::operator==(const EncryptedArray & other) const noexcept
{
    return (_elements == other._elements)
        && (*_public_element_ptr == *other._public_element_ptr);
}

void EncryptedArray::set_public_element(const mpz_class & x) noexcept
{
    auto result = public_elements.emplace(x);
    _public_element_ptr = result.first;
    _initialized = true;
}

EncryptedArray &
EncryptedArray::operator^=(const PlaintextArray & other) noexcept
{
    ASSERT(_initialized, "EncryptedArray must be initialized");

    const auto & public_element = *_public_element_ptr;

    const size_t n = min(_elements.size(), other._elements.size());

    // Do natural arithmetic operation modulo public element
    for (size_t i = 0; i < n; ++i) {
        _elements[i] += other._elements[i];
        _elements[i] %= public_element;
    }

    // If sizes don't match pad with zeros from the right
    for (size_t i = n; i < other._elements.size(); ++i) {
        _elements.push_back(other._elements[i]);
    }

    return *this;
}

EncryptedArray &
EncryptedArray::operator^=(const EncryptedArray & other) noexcept
{
    ASSERT(_initialized, "EncryptedArray must be initialized");

    const auto & public_element = *_public_element_ptr;

    _degree = max(_degree, other._degree);

    const size_t n = min(_elements.size(), other._elements.size());

    // Do natural arithmetic operation modulo public element
    for (size_t i = 0; i < n; ++i) {
        _elements[i] += other._elements[i];
        _elements[i] %= public_element;
    }

    // If sizes don't match pad with zeros from the right
    for (size_t i = n; i < other._elements.size(); ++i) {
        _elements.push_back(other._elements[i]);
    }

    return *this;
}

EncryptedArray &
EncryptedArray::operator&=(const PlaintextArray & other) noexcept
{
    ASSERT(_initialized, "EncryptedArray must be initialized");

    const auto & public_element = *_public_element_ptr;

    const size_t n = min(_elements.size(), other._elements.size());

    // Do natural arithmetic operation modulo public element
    for (size_t i = 0; i < n; ++i) {
        _elements[i] *= other._elements[i];
        _elements[i] %= public_element;
    }

    // If sizes don't match pad with ones from the right
    for (size_t i = n; i < other._elements.size(); ++i) {
        _elements.push_back(other._elements[i]);
    }

    return *this;
}

EncryptedArray &
EncryptedArray::operator&=(const EncryptedArray & other) noexcept
{
    ASSERT(_initialized, "EncryptedArray must be initialized");

    const auto & public_element = *_public_element_ptr;

    _degree = _degree + other._degree;

    const size_t n = min(_elements.size(), other._elements.size());

    // Do natural arithmetic operation modulo public element
    for (size_t i = 0; i < n; ++i) {
        _elements[i] *= other._elements[i];
        _elements[i] %= public_element;
    }

    // If sizes don't match pad with ones from the right
    for (size_t i = n; i < other._elements.size(); ++i) {
        _elements.push_back(other._elements[i]);
    }

    return *this;
}


const PlaintextArray
PlaintextArray::equal(const std::vector<PlaintextArray> & arrays) const noexcept
{
    ASSERT(arrays.size() > 0, "Input array must not be empty");

    PlaintextArray result {};

    for (const auto & array : arrays) {

        // Find difference (xor) between this and array
        const auto difference = *this ^ array;

        // Multiply (and) all elements of the difference array + 1
        // The result will decrypt to 1 iff all elements of this and array are equal
        bool all = 1;
        for (const auto & element : difference._elements)
        {
            all &= (element ^ 1);
        }

        result._elements.push_back(all);
    }

    return result;
}

const EncryptedArray
PlaintextArray::equal(const std::vector<EncryptedArray> & arrays) const noexcept
{
    ASSERT(arrays.size() > 0, "Input array must not be empty");

    const auto & public_element = arrays.front().public_element();

    EncryptedArray result( arrays.front().public_element()
                         , arrays.front().max_degree()
                         , arrays.front().degree()
                         );

    for (const auto & array : arrays) {

        // Find difference (xor) between this and array
        const auto difference = array ^ *this;

        // Multiply (and) all elements of the difference array + 1
        // The result will decrypt to 1 iff all elements of this and array are equal
        mpz_class all = 1;
        for (const auto & element : difference._elements)
        {
            all *= (element + 1);
            all %= public_element;
        }

        result._elements.push_back(all);

        // Set result degree to maximum degree of arrays
        auto current_degree = difference._degree * difference._elements.size();
        if (current_degree > result._degree) {
            result._degree = current_degree;
        }
    }

    return result;
}

const EncryptedArray
EncryptedArray::equal(const std::vector<PlaintextArray> & arrays) const noexcept
{
    ASSERT(_initialized, "EncryptedArray must be initialized");
    ASSERT(arrays.size() > 0, "Input array must not be empty");

    const auto & public_element = *_public_element_ptr;

    EncryptedArray result(public_element, _max_degree, _degree);

    for (const auto & array : arrays) {

        // Find difference (xor) between this and array
        const auto difference = *this ^ array;

        // Multiply (and) all elements of the difference array + 1
        // The result will decrypt to 1 iff all elements of this and array are equal
        mpz_class all = 1;
        for (const auto & element : difference._elements)
        {
            all *= (element + 1);
            all %= public_element;
        }

        result._elements.push_back(all);
    }

    return result;
}


const EncryptedArray
EncryptedArray::equal(const std::vector<EncryptedArray> & arrays) const noexcept
{
    ASSERT(_initialized, "EncryptedArray must be initialized");
    ASSERT(arrays.size() > 0, "Input array must not be empty");

    const auto & public_element = *_public_element_ptr;

    EncryptedArray result(public_element, _max_degree);

    for (const auto & array : arrays) {

        // Find difference (xor) between this and array
        const auto difference = *this ^ array;

        // Multiply (and) all elements of the difference array + 1
        // The result will decrypt to 1 iff all elements of this and array are equal
        mpz_class all = 1;
        for (const auto & element : difference._elements)
        {
            all *= (element + 1);
            all %= public_element;
        }

        result._elements.push_back(all);

        // Set result degree to maximum degree of arrays
        auto current_degree = difference._degree * difference._elements.size();
        if (current_degree > result._degree) {
            result._degree = current_degree;
        }
    }

    return result;
}

const PlaintextArray
PlaintextArray::select(const std::vector<PlaintextArray> & arrays) const noexcept
{
    ASSERT(arrays.size() > 0, "Input array must not be empty");

    PlaintextArray result {};

    for (size_t i = 0; i < min(_elements.size(), arrays.size()); ++i) {

        PlaintextArray selected {};

        // Multiply i-th element of this by all of the elements in array
        // The result will be decrypted to either original array or to array of zeros
        for (const auto & selected_element : arrays[i]._elements) {
            selected._elements.push_back(selected_element & _elements[i]);
        }

        result ^= selected;
    }

    return result;
}

const EncryptedArray
PlaintextArray::select(const std::vector<EncryptedArray> & arrays) const noexcept
{
    ASSERT(arrays.size() > 0, "Input array must not be empty");

    const auto & public_element = arrays.front().public_element();

    EncryptedArray result( arrays.front().public_element()
                         , arrays.front().max_degree()
                         , arrays.front().degree()
                         );

    for (size_t i = 0; i < min(_elements.size(), arrays.size()); ++i) {

        EncryptedArray selected(public_element, result._max_degree, arrays[i]._degree);

        // Multiply i-th element of this by all of the elements in array
        // The result will be decrypted to either original array or to array of zeros
        for (const auto & selected_element : arrays[i]._elements) {
            selected._elements.push_back((selected_element * _elements[i]) % public_element);
        }

        result ^= selected;
    }

    return result;
}

const EncryptedArray
EncryptedArray::select(const std::vector<PlaintextArray> & arrays) const noexcept
{
    ASSERT(_initialized, "EncryptedArray must be initialized");
    ASSERT(arrays.size() > 0, "Input array must not be empty");

    const auto & public_element = *_public_element_ptr;

    EncryptedArray result(public_element, _max_degree, _degree);

    for (size_t i = 0; i < min(_elements.size(), arrays.size()); ++i) {

        EncryptedArray selected(public_element, _max_degree, _degree);

        // Multiply i-th element of this by all of the elements in array
        // The result will be decrypted to either original array or to array of zeros
        for (const auto & selected_element : arrays[i]._elements) {
            selected._elements.push_back(selected_element * _elements[i]);
        }

        result ^= selected;
    }

    return result;
}

const EncryptedArray
EncryptedArray::select(const std::vector<EncryptedArray> & arrays) const noexcept
{
    ASSERT(_initialized, "EncryptedArray must be initialized");
    ASSERT(arrays.size() > 0, "Input array must not be empty");

    const auto & public_element = *_public_element_ptr;

    EncryptedArray result(public_element, _max_degree);

    for (size_t i = 0; i < min(_elements.size(), arrays.size()); ++i) {

        EncryptedArray selected(public_element, _max_degree);

        // Multiply i-th element of this by all of the elements in array
        // The result will be decrypted to either original array or to array of zeros
        for (const auto & selected_element : arrays[i]._elements) {
            selected._elements.push_back((selected_element * _elements[i]) % public_element);
        }

        selected._degree =  _degree + arrays[i]._degree;

        result ^= selected;
    }

    return result;
}

EncryptedArray &
EncryptedArray::extend(const EncryptedArray & other) noexcept
{
    ASSERT(_initialized, "EncryptedArray must be initialized");

    for (const auto & element : other._elements) {
        _elements.push_back(element);
    }

    _degree = max(_degree, other._degree);

    return *this;
}

const mpz_class &
EncryptedArray::public_element() const noexcept
{
    ASSERT(_initialized, "EncryptedArray must be initialized");

    return *_public_element_ptr;
}

PlaintextArray
sum(const vector<PlaintextArray> & arrays) noexcept
{
    ASSERT(arrays.size() > 0, "Input array must not be empty");

    PlaintextArray result {};

    for (const auto & array : arrays) {
        result ^= array;
    }

    return result;
}

EncryptedArray
sum(const vector<EncryptedArray> & arrays) noexcept
{
    ASSERT(arrays.size() > 0, "Input array must not be empty");

    EncryptedArray result( arrays.front().public_element()
                         , arrays.front().max_degree()
                         , arrays.front().degree()
                         );

    for (const auto & array : arrays) {
        result ^= array;
    }

    return result;
}

PlaintextArray
product(const vector<PlaintextArray> & arrays) noexcept
{
    ASSERT(arrays.size() > 0, "Input array must not be empty");

    PlaintextArray result {};

    for (const auto & array : arrays) {
        result &= array;
    }

    return result;
}

EncryptedArray
product(const vector<EncryptedArray> & arrays) noexcept
{
    ASSERT(arrays.size() > 0, "Input array must not be empty");

    EncryptedArray result( arrays.front().public_element()
                         , arrays.front().max_degree()
                         , 0);

    for (const auto & array : arrays) {
        result &= array;
    }

    return result;
}

PlaintextArray
concat(const vector<PlaintextArray> & arrays) noexcept
{
    ASSERT(arrays.size() > 0, "Input array must not be empty");

    PlaintextArray result {};

    for (const auto & array : arrays) {
        result.extend(array);
    }

    return result;
}

EncryptedArray
concat(const vector<EncryptedArray> & arrays) noexcept
{
    ASSERT(arrays.size() > 0, "Input array must not be empty");

    EncryptedArray result( arrays.front().public_element()
                         , arrays.front().max_degree()
                         , arrays.front().degree()
                         );

    for (const auto & array : arrays) {
        result.extend(array);
    }

    return result;
}

CompressedCiphertext::CompressedCiphertext(const ParameterSet & params) noexcept :
  _parameter_set(params)
{
    initialize_prf_stream();
}

void CompressedCiphertext::initialize_prf_stream() const noexcept
{
    _prf_stream.reset(
        new PseudoRandomStream{_parameter_set.ciphertext_size_bits, _parameter_set.prf_seed});
}

bool CompressedCiphertext::operator==(const CompressedCiphertext & other) const noexcept
{
    return (_parameter_set == other._parameter_set)
        && (_public_element_delta == other._public_element_delta)
        && (_elements_deltas == other._elements_deltas);
}

EncryptedArray CompressedCiphertext::expand() const noexcept
{
    _prf_stream->reset();

    // Restore public element
    const auto & prf_output = _prf_stream->next();
    const auto public_element = prf_output - _public_element_delta;

    EncryptedArray result(public_element, _parameter_set.degree());

    // Restore ciphertext elements
    for (const auto & delta : _elements_deltas) {
        const auto & prf_output = _prf_stream->next();
        result._elements.push_back(prf_output - delta);
    }

    return result;
}

} // namespace she
