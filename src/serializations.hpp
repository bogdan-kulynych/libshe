#pragma once
#include <string>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/multiprecision/gmp.hpp>

#include <gmpxx.h>

#include "defs.hpp"


namespace boost { namespace serialization {

template<class Archive>
void save(Archive & ar, const ::boost::multiprecision::backends::gmp_int & value, const unsigned int version)
{
    std::string repr = mpz_class(value.data()).get_str(she::INTEGER_SERIALIZATION_BASE);
}

template<class Archive>
void load(Archive & ar, ::boost::multiprecision::backends::gmp_int & value, const unsigned int version)
{
    std::string repr;
    mpz_class gmp_class_value;

    ar & make_nvp("repr", repr);
    gmp_class_value.set_str(repr, she::INTEGER_SERIALIZATION_BASE);

    value = gmp_class_value.get_mpz_t();
}

}} // namespace boost::serialization

BOOST_SERIALIZATION_SPLIT_FREE(boost::multiprecision::backends::gmp_int);
