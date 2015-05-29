#pragma once

#include <cstddef>
#include <set>
#include <vector>
#include <memory>

#include <boost/operators.hpp>
#include <boost/variant/variant.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/split_free.hpp>

#include <gmpxx.h>

#include "random.hpp"
#include "serializations.hpp"
#include "key.hpp"


namespace she
{

class PlaintextArray;
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


class EncryptedArray : boost::equality_comparable<EncryptedArray
                     , boost::xorable<EncryptedArray
                     , boost::xorable<EncryptedArray, PlaintextArray
                     , boost::andable<EncryptedArray
                     , boost::andable<EncryptedArray, PlaintextArray
                     > > > > >
{
 friend class PlaintextArray;
 friend class CompressedCiphertext;
 public:
    // Construct empty ciphertext with given public element
    EncryptedArray(const mpz_class & x, unsigned int max_degree, unsigned int degree=1) noexcept;

    // Empty ctor for deserialization purposes
    EncryptedArray() noexcept {};

    // Homomorphic element-wise addition (XOR)
    EncryptedArray & operator^=(const PlaintextArray &) noexcept;
    EncryptedArray & operator^=(const EncryptedArray &) noexcept;

    // Homomorphic element-wise multiplication (AND)
    EncryptedArray & operator&=(const PlaintextArray &) noexcept;
    EncryptedArray & operator&=(const EncryptedArray &) noexcept;

    // Homomorphic equality comparison
    const EncryptedArray equal(const std::vector<PlaintextArray> &) const noexcept;
    const EncryptedArray equal(const std::vector<EncryptedArray> &) const noexcept;

    // Homomorphic select function
    const EncryptedArray select(const std::vector<PlaintextArray> &) const noexcept;
    const EncryptedArray select(const std::vector<EncryptedArray> &) const noexcept;

    // Extend array
    EncryptedArray & extend(const EncryptedArray & other) noexcept;

    // Reflects how noisy the ciphertexts, equals the number of homomorphic multiplications performed since encryption
    unsigned int degree() const noexcept { return _degree; }

    // Approximate maximum number of homomorphic multiplications
    unsigned int max_degree() const noexcept { return _max_degree; }

    // Ciphertext size
    size_t size() const noexcept { return _elements.size(); }

    // Encrypted bits
    const std::vector<mpz_class>& elements() const noexcept { return _elements; }
    std::vector<mpz_class>& elements() noexcept { return _elements; }

    // Public element used in homomorphic operations
    const mpz_class & public_element() const noexcept;

    // Representation comparison (non-homomorphic)
    bool operator==(const EncryptedArray &) const noexcept;

 private:
    unsigned int _degree;
    unsigned int _max_degree;

    std::vector<mpz_class> _elements;

    void set_public_element(const mpz_class & x) noexcept;
    static std::set<mpz_class> public_elements;
    typename std::set<mpz_class>::const_iterator _public_element_ptr;

    bool _initialized;

 private:
    friend class boost::serialization::access;

    template<class Archive>
    void save(Archive & ar, unsigned int const version) const
    {
        ar & BOOST_SERIALIZATION_NVP(_degree);
        ar & BOOST_SERIALIZATION_NVP(_max_degree);
        ar & BOOST_SERIALIZATION_NVP(_elements);
        ar & boost::serialization::make_nvp("_public_element", *_public_element_ptr);
    }

    template<class Archive>
    void load(Archive & ar, unsigned int const version)
    {

        ar & BOOST_SERIALIZATION_NVP(_degree);
        ar & BOOST_SERIALIZATION_NVP(_max_degree);
        ar & BOOST_SERIALIZATION_NVP(_elements);

        mpz_class x;
        ar & boost::serialization::make_nvp("_public_element", x);
        set_public_element(x);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
};

// Homomorphic addition (XOR)
PlaintextArray sum(const std::vector<PlaintextArray> &) noexcept;
EncryptedArray sum(const std::vector<EncryptedArray> &) noexcept;

// Homomorphic multiplication (AND)
PlaintextArray product(const std::vector<PlaintextArray> &) noexcept;
EncryptedArray product(const std::vector<EncryptedArray> &) noexcept;

// Arrays concatenation
PlaintextArray concat(const std::vector<PlaintextArray> &) noexcept;
EncryptedArray concat(const std::vector<EncryptedArray> &) noexcept;


class CompressedCiphertext : boost::equality_comparable<CompressedCiphertext>
{
 friend class PrivateKey;
 public:
    // Empty ctor for deserialization purposes
    CompressedCiphertext() noexcept {};

    // Expand ciphertext
    EncryptedArray expand() const noexcept;

    // Ciphertext size
    size_t size() const noexcept { return _elements_deltas.size(); }

    // Compressions of encrypted bits
    const std::vector<mpz_class> & elements_deltas() const noexcept { return _elements_deltas; }
    const mpz_class & public_element_delta() const noexcept { return _public_element_delta; }

    // Representation comparison
    bool operator== (const CompressedCiphertext &) const noexcept;

 private:
    CompressedCiphertext(const ParameterSet & params) noexcept;

    ParameterSet _parameter_set;

    void initialize_oracle() const noexcept;
    mutable std::unique_ptr<RandomOracle> _oracle;

    std::vector<mpz_class> _elements_deltas;
    mpz_class _public_element_delta;

 private:
    friend class boost::serialization::access;

    template<class Archive>
    void save(Archive & ar, unsigned int const version) const
    {
        ar & BOOST_SERIALIZATION_NVP(_parameter_set);
        ar & BOOST_SERIALIZATION_NVP(_elements_deltas);
        ar & BOOST_SERIALIZATION_NVP(_public_element_delta);
    }

    template<class Archive>
    void load(Archive & ar, unsigned int const version)
    {
        ar & BOOST_SERIALIZATION_NVP(_parameter_set);
        ar & BOOST_SERIALIZATION_NVP(_elements_deltas);
        ar & BOOST_SERIALIZATION_NVP(_public_element_delta);

        initialize_oracle();
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
};

} // namespace she
