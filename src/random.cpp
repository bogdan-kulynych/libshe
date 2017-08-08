#include <random>

#include "she/random.hpp"
#include "she/defs.hpp"

using std::out_of_range;
using std::random_device;
using std::make_pair;
using std::map;
using std::pair;
using std::vector;


namespace she
{

CSPRNG::CSPRNG() noexcept : _generator(gmp_randinit_default)
{
    // Generate 64-bit seed
    random_device dev(RANDOM_DEVICE);
    mpz_class seed32 = dev();
    _generator.seed(seed32 << 32 | dev());
}

mpz_class
CSPRNG::get_bits(unsigned int bits) const noexcept
{
    return _generator.get_z_bits(bits);
}

mpz_class
CSPRNG::get_range_bits(unsigned int bits) const noexcept
{
    return _generator.get_z_range(mpz_class(1) << bits);;
}

mpz_class
CSPRNG::get_range(const mpz_class & upper_bound) const noexcept
{
    return _generator.get_z_range(upper_bound);
}


PseudoRandomStream::PseudoRandomStream(unsigned int size, unsigned int seed) :
  _size(size),
  _seed(seed),
  _generator(gmp_randinit_default),
  _current_value(0)
{
    _generator.seed(seed);
}

map<PseudoRandomStream::keys_t, PseudoRandomStream::values_t> PseudoRandomStream::cached_values = {};

const mpz_class & PseudoRandomStream::next() const noexcept
{
    keys_t context{_size, _seed};
    auto it = PseudoRandomStream::cached_values.find(context);

    if (it != cached_values.end()) {
        if (it->second.size() <= _current_value) {
            it->second.push_back(_generator.get_z_bits(_size));
        }
    } else {
        cached_values[context] = vector<mpz_class>{_generator.get_z_bits(_size)};
    }

    return cached_values[context][_current_value++];
}

const void PseudoRandomStream::reset() const noexcept
{
    _current_value = 0;
}

}
