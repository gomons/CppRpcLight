#pragma once

#include <tuple>
#include <cstdint>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

namespace boost {
namespace serialization {

    template<uint32_t N>
    struct Serialize
    {
        template<class Archive, typename... Args>
        static void serialize(Archive &ar, std::tuple<Args...> &t, const unsigned int version)
        {
            ar & std::get<N-1>(t);
            Serialize<N-1>::serialize(ar, t, version);
        }
    };

    template<>
    struct Serialize<0>
    {
        template<class Archive, typename... Args>
        static void serialize(Archive & ar, std::tuple<Args...> &t, const unsigned int version)
        {}
    };

    template<class Archive, typename... Args>
    void serialize(Archive &ar, std::tuple<Args...> &t, const unsigned int version)
    {
        Serialize<sizeof...(Args)>::serialize(ar, t, version);
    }

} // namespace serialization
} // namespace boost