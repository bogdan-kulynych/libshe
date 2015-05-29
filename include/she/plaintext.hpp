#pragma once

#include <cstddef>
#include <memory>

#include <boost/operators.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/split_free.hpp>

#include "serializations.hpp"


namespace she
{

class EncryptedArray;

class PlaintextArray : boost::equality_comparable<PlaintextArray
                     , boost::xorable<PlaintextArray
                     , boost::andable<PlaintextArray
                     > > >
{
 friend class EncryptedArray;
 public:
    // Construct from plaintext
    PlaintextArray(const std::vector<bool> & plaintext) noexcept : _elements(plaintext) {}

    // Empty ctor for deserialization purposes
    PlaintextArray() noexcept {};

    // Convert to bits
    operator const std::vector<bool>() const { return _elements; };
    operator std::vector<bool>() { return _elements; };

    // Element-wise addition (XOR)
    PlaintextArray & operator^=(const PlaintextArray &) noexcept;

    // Element-wise multiplication (AND)
    PlaintextArray & operator&=(const PlaintextArray &) noexcept;

    // Homomorphic equality comparison
    const PlaintextArray equal(const std::vector<PlaintextArray> &) const noexcept;
    const EncryptedArray equal(const std::vector<EncryptedArray> &) const noexcept;

    // Homomorphic select function
    const PlaintextArray select(const std::vector<PlaintextArray> &) const noexcept;
    const EncryptedArray select(const std::vector<EncryptedArray> &) const noexcept;

    // Extend array
    PlaintextArray & extend(const PlaintextArray & other) noexcept;

    // EncryptedArray compatibility
    unsigned int degree() const noexcept { return 0; }
    unsigned int max_degree() const noexcept { return 0; }

    // Ciphertext size
    size_t size() const noexcept { return _elements.size(); }

    // Encrypted bits
    const std::vector<bool>& elements() const noexcept { return _elements; }
    std::vector<bool>& elements() noexcept { return _elements; }

    // Representation comparison
    bool operator==(const PlaintextArray &) const noexcept;

 private:
    friend class boost::serialization::access;

    std::vector<bool> _elements;

    template<class Archive>
    void serialize(Archive & ar, unsigned int const version) const
    {
        ar & BOOST_SERIALIZATION_NVP(_elements);
    }
};

} // namespace she