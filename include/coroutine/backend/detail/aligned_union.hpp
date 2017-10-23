#ifndef COROUTINE_BACKEND_DETAIL_ALIGNED_UNION_HPP
#define COROUTINE_BACKEND_DETAIL_ALIGNED_UNION_HPP

#include <type_traits>

namespace coro
{

namespace back
{

namespace detail
{

namespace
{
template<typename T>
constexpr T max(T lhs, T rhs)
{
    return (lhs >= rhs) ? lhs : rhs;
}

template<typename T>
constexpr T max(T value)
{
    return value;
}

template<typename Head, typename Second, typename... Tail>
constexpr Head max(Head head, Second second, Tail... tail)
{
    return max(head, max(second, tail...));
}

}

template<std::size_t MinSize, typename... Ts>
using aligned_union = std::aligned_storage<max(sizeof(Ts)...), max(alignof(Ts)...)>;

}

}

}

#endif // COROUTINE_BACKEND_DETAIL_ALIGNED_UNION_HPP
