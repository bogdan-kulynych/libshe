#include <random>

#include "random.hpp"
#include "defs.hpp"

using std::out_of_range;
using std::random_device;
using std::make_pair;
using std::map;
using std::pair;
using std::vector;

using boost::multiprecision::mpz_int;


namespace she
{

CSPRNG::CSPRNG() : _generator(gmp_randinit_default)
{
    random_device dev(RANDOM_DEVICE);
    _generator.seed(dev());
}

boost::multiprecision::mpz_int
CSPRNG::get_bits(unsigned int bits) const
{
    const mpz_class output = _generator.get_z_bits(bits);
    return output.get_mpz_t();
}

boost::multiprecision::mpz_int
CSPRNG::get_range_bits(unsigned int bits) const
{
    const mpz_class output = _generator.get_z_range(mpz_class(1) << bits);
    return output.get_mpz_t();
}

boost::multiprecision::mpz_int
CSPRNG::get_range(const boost::multiprecision::mpz_int & upper_bound) const
{
    const mpz_class output = _generator.get_z_range(upper_bound.convert_to<mpz_class>());
    return output.get_mpz_t();
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

const mpz_int & RandomOracle::next() const
{
    keys_t context{_size, _seed};
    auto it = RandomOracle::cached_values.find(context);

    if (it != cached_values.end()) {
        if (it->second.size() <= _current_value) {
            mpz_class raw_output = _generator.get_z_bits(_size);
            mpz_int output = mpz_int(raw_output.get_mpz_t());
            it->second.push_back(output);
        }
    } else {
        mpz_class raw_output = _generator.get_z_bits(_size);
        mpz_int output = mpz_int(raw_output.get_mpz_t());
        cached_values[context] = vector<mpz_int>{output};
    }

    return cached_values[context][_current_value++];
}

const void RandomOracle::reset() const
{
    _current_value = 0;
}

};
