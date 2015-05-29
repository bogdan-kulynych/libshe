#include <cmath>

#include "she.hpp"
#include "she/exceptions.hpp"

using std::min;
using std::max;
using std::vector;


namespace she
{

PlaintextArray & PlaintextArray::operator^=(const PlaintextArray & other) noexcept
{
    const size_t n = min(_elements.size(), other._elements.size());

    // Do bit-wise XOR
    for (size_t i = 0; i < n; ++i) {
        _elements[i] = _elements[i] ^ other._elements[i];
    }

    // If sizes don't match pad with zeros from the right
    for (size_t i = n; i < other._elements.size(); ++i) {
        _elements.push_back(other._elements[i]);
    }

    return *this;
}

PlaintextArray & PlaintextArray::operator&=(const PlaintextArray & other) noexcept
{
    const size_t n = min(_elements.size(), other._elements.size());

    // Do bit-wise AND
    for (size_t i = 0; i < n; ++i) {
        _elements[i] = _elements[i] & other._elements[i];
    }

    // If sizes don't match pad with ones from the right
    for (size_t i = n; i < other._elements.size(); ++i) {
        _elements.push_back(other._elements[i]);
    }

    return *this;
}

PlaintextArray & PlaintextArray::extend(const PlaintextArray & other) noexcept
{
    for (const auto & element : other._elements) {
        _elements.push_back(element);
    }

    return *this;
}

bool PlaintextArray::operator==(const PlaintextArray & other) const noexcept
{
    return _elements == other._elements;
}

} // namespace she