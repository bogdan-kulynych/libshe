#pragma once

#include <cstddef>
#include <string>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_free.hpp>

#include <gmpxx.h>

#include "she/defs.hpp"


namespace boost { namespace serialization {

template<class Archive>
void save(Archive & ar, const mpz_class & value, const unsigned int version)
{
    std::string repr = value.get_str(she::INTEGER_SERIALIZATION_BASE);
    ar & BOOST_SERIALIZATION_NVP(repr);
}

template<class Archive>
void load(Archive & ar, mpz_class & value, const unsigned int version)
{
    std::string repr;
    ar & BOOST_SERIALIZATION_NVP(repr);
    value.set_str(repr, she::INTEGER_SERIALIZATION_BASE);
}

}} // namespace boost::serialization

BOOST_SERIALIZATION_SPLIT_FREE(mpz_class)
