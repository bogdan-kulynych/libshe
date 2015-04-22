#include <random>

#include "random.hpp"
#include "defs.hpp"

using std::out_of_range;
using std::random_device;
using std::make_pair;
using std::map;
using std::pair;
using std::vector;


namespace she
{

CSPRNG::CSPRNG() : _generator(gmp_randinit_default)
{
    random_device dev(RANDOM_DEVICE);
    _generator.seed(dev());
}

mpz_class
CSPRNG::get_bits(unsigned int bits) const
{
    return _generator.get_z_bits(bits);
}

mpz_class
CSPRNG::get_range_bits(unsigned int bits) const
{
    return _generator.get_z_range(mpz_class(1) << bits);;
}

mpz_class
CSPRNG::get_range(const mpz_class & upper_bound) const
{
    return _generator.get_z_range(upper_bound);
}


RandomOracle::RandomOracle(unsigned int size, unsigned int seed) :
  _size(size),
  _seed(seed),
  _generator(gmp_randinit_default),
  _current_value(0)
{
    _generator.seed(seed);
};

map<RandomOracle::keys_t, RandomOracle::values_t> RandomOracle::cached_values = {};

const mpz_class & RandomOracle::next() const
{
    keys_t context{_size, _seed};
    auto it = RandomOracle::cached_values.find(context);

    if (it != cached_values.end()) {
        if (it->second.size() <= _current_value) {
            it->second.push_back(_generator.get_z_bits(_size));
        }
    } else {
        cached_values[context] = vector<mpz_class>{_generator.get_z_bits(_size)};
    }

    return cached_values[context][_current_value++];
}

const void RandomOracle::reset() const
{
    _current_value = 0;
}

};
