#ifndef COROUTINE_COROUTINE_HPP
#define COROUTINE_COROUTINE_HPP

namespace coro
{

namespace detail
{


template<typename T>
class push_coroutine
{
public:
    template<typename Value>
    void push(Value&& value)
    {
        
    }
};

}

template<typename T>
struct coroutine
{
    using push_coroutine = coro::detail::push_coroutine<T>;:NERD
}

}

#endif // COROUTINE_COROUTINE_HPP
