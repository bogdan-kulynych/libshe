#pragma once

#include <cstddef>
#include <vector>
#include <utility>
#include <map>

#include <gmpxx.h>


namespace she
{

class CSPRNG
{
 public:
    CSPRNG() noexcept;

    mpz_class get_bits(unsigned int bits) const noexcept;
    mpz_class get_range_bits(unsigned int bits) const noexcept;
    mpz_class get_range(const mpz_class & upper_bound) const noexcept;

 private:
    mutable gmp_randclass _generator;
};


class PseudoRandomStream
{
 public:
    PseudoRandomStream(unsigned int size, unsigned int seed);

    const mpz_class & next() const noexcept;
    const void reset() const noexcept;
    static void reset_cache() noexcept { cached_values.clear(); }

 private:
    unsigned int _size;
    unsigned int _seed;
    mutable gmp_randclass _generator;

    mutable size_t _current_value;

    using keys_t = std::pair<unsigned int, unsigned int>;
    using values_t = std::vector<mpz_class>;

    static std::map<keys_t, values_t> cached_values;
};

} // namespace she
