#pragma once

#include <cstddef>
#include <vector>
#include <memory>

#include <boost/operators.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>

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

    ParameterSet() noexcept;

    unsigned int security;
    unsigned int noise_size_bits;
    unsigned int private_key_size_bits;
    unsigned int ciphertext_size_bits;
    unsigned int oracle_seed;

    // Generate parameter set for given `security`, random oracle `seed`, that allows to perform at least
    // `circuit_mult_size homomorphic multiplications on ciphertexts
    static const ParameterSet
    generate_parameter_set(unsigned int security, unsigned int circuit_mult_size, unsigned int seed);

    // Approximate number of homomorphic multiplications that can be performed
    unsigned int degree() const noexcept { return private_key_size_bits / noise_size_bits; }

    bool operator==(const ParameterSet &) const noexcept;

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


class CompressedCiphertext;
class HomomorphicArray;

class PrivateKey : boost::equality_comparable<PrivateKey>
{
 public:
    // Constuct private from parameter set
    PrivateKey(const ParameterSet &) noexcept;

    // Empty ctor for deserialization purposes
    PrivateKey() noexcept {};

    // Produce a compressed ciphertext from a plaintext
    CompressedCiphertext encrypt(const std::vector<bool> & bits) const noexcept;

    // Decrypt an expanded ciphertext
    std::vector<bool> decrypt(const HomomorphicArray &) const noexcept;

    const ParameterSet & parameter_set() const noexcept { return _parameter_set; };
    const mpz_class & private_element() const noexcept { return _private_element; }

    bool operator==(const PrivateKey &) const noexcept;

 private:
    ParameterSet _parameter_set;

    void initialize_random_generators() const noexcept;
    mutable std::unique_ptr<CSPRNG> _generator;
    mutable std::unique_ptr<RandomOracle> _oracle;

    PrivateKey& generate_values() noexcept;
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

    BOOST_SERIALIZATION_SPLIT_MEMBER()
};


} // namespace she