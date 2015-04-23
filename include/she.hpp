#pragma once
#include <set>
#include <vector>
#include <memory>

#include <boost/operators.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/split_free.hpp>

#include <gmpxx.h>

#include "random.hpp"
#include "serializations.hpp"


namespace she
{

class ParameterSet : boost::equality_comparable<ParameterSet>
{
public:
    ParameterSet(unsigned int security,
                 unsigned int noise_size_bits,
                 unsigned int private_key_size_bits,
                 unsigned int ciphertext_size_bits,
                 unsigned int oracle_seed);

    ParameterSet();

    unsigned int security;
    unsigned int noise_size_bits;
    unsigned int private_key_size_bits;
    unsigned int ciphertext_size_bits;
    unsigned int oracle_seed;

    static const ParameterSet
    generate_parameter_set(unsigned int security, unsigned int circuit_mult_size, unsigned int seed);

    bool operator==(const ParameterSet &) const;

 private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, unsigned int const version)
    {
        ar & BOOST_SERIALIZATION_NVP(security);
        ar & BOOST_SERIALIZATION_NVP(noise_size_bits);
        ar & BOOST_SERIALIZATION_NVP(private_key_size_bits);
        ar & BOOST_SERIALIZATION_NVP(ciphertext_size_bits);
        ar & BOOST_SERIALIZATION_NVP(oracle_seed);
    }
};


class HomomorphicArray : boost::equality_comparable<HomomorphicArray,
                         boost::xorable<HomomorphicArray,
                         boost::andable<HomomorphicArray
                         > > >
{
 friend class CompressedCiphertext;
 public:
    // Construct from plaintext
    explicit HomomorphicArray(std::vector<bool> plaintext);

    // Empty ctor for deserialization purposes
    HomomorphicArray() {};

    // Homomorphic addition (XOR)
    const HomomorphicArray operator^=(const HomomorphicArray &) const;

    // Homomorphic multiplication (AND)
    const HomomorphicArray operator&=(const HomomorphicArray &) const;

    // Homomorphic equality comparison
    const HomomorphicArray equal(const HomomorphicArray &) const;

    // Homomorphic select function
    const HomomorphicArray select(const std::vector<HomomorphicArray> & mat) const;

    // HomomorphicArray representation comparison (non-homomorphic)
    const bool operator==(const HomomorphicArray &) const;

    HomomorphicArray& concat(const HomomorphicArray & other);
    const HomomorphicArray concat(const HomomorphicArray & other) const;

    size_t size() const { return _elements.size(); }

    const std::vector<mpz_class>& elements() const { return _elements; }
    std::vector<mpz_class>& elements() { return _elements; }

    const mpz_class & public_element() const { return *_public_element_ptr; }

 private:
    HomomorphicArray(const std::vector<mpz_class> & array, const mpz_class & x0);

    std::vector<mpz_class> _elements;

    void set_public_element(const mpz_class & x);
    static std::set<mpz_class> public_elements;
    typename std::set<mpz_class>::const_iterator _public_element_ptr;

 private:
    friend class boost::serialization::access;

    template<class Archive>
    void save(Archive & ar, unsigned int const version) const
    {
        ar & BOOST_SERIALIZATION_NVP(_elements);
        ar & boost::serialization::make_nvp("_public_element", *_public_element_ptr);
    }

    template<class Archive>
    void load(Archive & ar, unsigned int const version)
    {
        ar & BOOST_SERIALIZATION_NVP(_elements);
        mpz_class x;
        ar & boost::serialization::make_nvp("_public_element", x);
        set_public_element(x);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER();
};

// Optimized homomorphic addition (XOR)
HomomorphicArray& sum(const std::vector<HomomorphicArray> &);

// Optimized homomorphic multiplication (AND)
HomomorphicArray& product(const std::vector<HomomorphicArray> &);

// Optimized ciphertexts concatenation
HomomorphicArray& concat(const std::vector<HomomorphicArray> &);


class CompressedCiphertext : boost::equality_comparable<CompressedCiphertext>
{
 friend class PrivateKey;
 public:
    // Empty ctor for deserialization purposes
    CompressedCiphertext() {};

    // Expand ciphertext
    HomomorphicArray expand() const;

    const std::vector<mpz_class> & elements_deltas() const { return _elements_deltas; }
    const mpz_class & public_element_delta() const { return _public_element_delta; }

 private:
    CompressedCiphertext(const ParameterSet & params);

    ParameterSet _parameter_set;

    void initialize_oracle() const;
    mutable std::unique_ptr<RandomOracle> _oracle;

    std::vector<mpz_class> _elements_deltas;
    mpz_class _public_element_delta;

 private:
    friend class boost::serialization::access;

    template<class Archive>
    void save(Archive & ar, unsigned int const version) const
    {
        ar & BOOST_SERIALIZATION_NVP(_elements_deltas);
        ar & BOOST_SERIALIZATION_NVP(_public_element_delta);
    }

    template<class Archive>
    void load(Archive & ar, unsigned int const version)
    {
        ar & BOOST_SERIALIZATION_NVP(_elements_deltas);
        ar & BOOST_SERIALIZATION_NVP(_public_element_delta);

        initialize_oracle();
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER();
};


class PrivateKey : boost::equality_comparable<PrivateKey>
{
 public:
    // Constuct private from parameter set
    PrivateKey(const ParameterSet &);

    // Empty ctor for deserialization purposes
    PrivateKey() {};

    CompressedCiphertext encrypt(const std::vector<bool> & bits) const;
    std::vector<bool> decrypt(const HomomorphicArray &) const;

    const ParameterSet & parameter_set() const { return _parameter_set; };
    const mpz_class & private_element() const { return _private_element; }

    bool operator==(const PrivateKey &) const;

 private:
    ParameterSet _parameter_set;

    void initialize_random_generators() const;
    mutable std::unique_ptr<CSPRNG> _generator;
    mutable std::unique_ptr<RandomOracle> _oracle;

    PrivateKey& generate_values();
    mpz_class _private_element;

 private:
    friend class boost::serialization::access;

    template<class Archive>
    void save(Archive & ar, unsigned int const version) const
    {
        ar & BOOST_SERIALIZATION_NVP(_parameter_set);
        ar & BOOST_SERIALIZATION_NVP(_private_element);
    }

    template<class Archive>
    void load(Archive & ar, unsigned int const version)
    {
        ar & BOOST_SERIALIZATION_NVP(_parameter_set);
        ar & BOOST_SERIALIZATION_NVP(_private_element);

        initialize_random_generators();
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER();
};


} // namespace she
