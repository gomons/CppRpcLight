#pragma once

#include <type_traits>

namespace cpp_rpc_light
{
    template<typename F, typename Tuple, bool Enough, int TotalArgs, int...N>
    struct call_impl
    {
        auto static call(F f, Tuple&& t)
        {
            return call_impl<F, Tuple, TotalArgs == 1 + sizeof...(N), TotalArgs, N..., sizeof...(N)>::call(f, std::forward<Tuple>(t));
        }
    };

    template<typename F, typename Tuple, int TotalArgs, int... N>
    struct call_impl<F, Tuple, true, TotalArgs, N...>
    {
        auto static call(F f, Tuple&& t)
        {
            return f(std::get<N>(std::forward<Tuple>(t))...);
        }
    };

    template<typename F, typename Tuple>
    auto call(F f, Tuple&& t)
    {
        typedef typename std::decay<Tuple>::type type;
        return call_impl<F, Tuple, std::tuple_size<type>::value == 0, std::tuple_size<type>::value>::call(f, std::forward<Tuple>(t));
    }
} // namespace cpp_rpc_light