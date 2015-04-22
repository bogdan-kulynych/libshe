#include "../src/serializations.hpp"

#include <boost/mpl/list.hpp>

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

using boost::archive::xml_oarchive;
using boost::archive::xml_iarchive;
using boost::archive::text_oarchive;
using boost::archive::text_iarchive;


template <class InputArchive, class OutputArchive>
struct Format {
    using iarchive = InputArchive;
    using oarchive = OutputArchive;
};

using Formats = boost::mpl::list< Format<text_iarchive, text_oarchive> >;
