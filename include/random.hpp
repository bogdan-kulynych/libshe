#pragma once
#include <vector>
#include <utility>
#include <map>

#include <boost/multiprecision/gmp.hpp>

#include <gmpxx.h>


namespace she
{

class CSPRNG
{
 public:
    CSPRNG();

    boost::multiprecision::mpz_int get_bits(unsigned int bits) const;
    boost::multiprecision::mpz_int get_range_bits(unsigned int bits) const;
    boost::multiprecision::mpz_int get_range(const boost::multiprecision::mpz_int & upper_bound) const;

 private:
    mutable gmp_randclass _generator;
};


class RandomOracle
{
 public:
    RandomOracle(unsigned int size, unsigned int seed);

    const boost::multiprecision::mpz_int & next() const;
    const void reset() const;
    static void reset_cache() { cached_values.clear(); }

 private:
    unsigned int _size;
    unsigned int _seed;
    mutable gmp_randclass _generator;

    mutable size_t _current_value;

    using keys_t = std::pair<unsigned int, unsigned int>;
    using values_t = std::vector<boost::multiprecision::mpz_int>;

    static std::map<keys_t, values_t> cached_values;
};

} // namespace she
