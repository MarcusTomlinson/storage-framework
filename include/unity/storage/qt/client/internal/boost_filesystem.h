#pragma once

#include <boost/version.hpp>

#if BOOST_VERSION / 100000 == 1 && BOOST_VERSION / 100 & 1000 <= 55
    #define BOOST_NO_CXX11_SCOPED_ENUMS
    #include <boost/filesystem.hpp>
    #undef BOOST_NO_CXX11_SCOPED_ENUMS
#else
    #include <boost/filesystem.hpp>
#endif
