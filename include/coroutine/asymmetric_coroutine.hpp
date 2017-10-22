#ifndef COROUTINE_ASYMMETRIC_COROUTINE_HPP
#define COROUTINE_ASYMMETRIC_COROUTINE_HPP

#include <coroutine/backend/asymmetric_coroutine.hpp>

namespace coro
{

template<typename T>
struct asymmetric_coroutine
{
    using pull_type = coro::back::asymmetric_coroutine::pull_type<T>;
    using push_type = coro::back::asymmetric_coroutine::push_type<T>;
};

}

#endif // COROUTINE_ASYMMETRIC_COROUTINE_HPP
