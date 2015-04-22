#pragma once
#include <string>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_free.hpp>

#include <gmpxx.h>

#include "defs.hpp"


namespace boost { namespace serialization {

template<class Archive>
void save(Archive & ar, const mpz_class & value, const unsigned int version)
{
    std::string repr = value.get_str(she::INTEGER_SERIALIZATION_BASE);
}

template<class Archive>
void load(Archive & ar, mpz_class & value, const unsigned int version)
{
    std::string repr;
    ar & make_nvp("repr", repr);
    value.set_str(repr, she::INTEGER_SERIALIZATION_BASE);
}

}} // namespace boost::serialization

BOOST_SERIALIZATION_SPLIT_FREE(mpz_class);
