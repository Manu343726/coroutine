#ifndef COROUTINE_BACKEND_COROUTINE_HPP
#define COROUTINE_BACKEND_COROUTINE_HPP

#include <coroutine/backend/context_pool.hpp>
#include <functional>
#include <cassert>

namespace coro
{

namespace back
{

template<typename T>
class value_channel
{
public:
    template<typename Value>
    void set(Value&& value)
    {
        _value = std::forward<Value>(value);
    }

    const T& get() const
    {
        return _value;
    }

private:
    T _value;
};

template<typename T>
class coroutine
{
public:
    using function = std::function<void(coroutine& coro)>;

    coroutine(function coro_function) :
        _pool{&get_context_pool()},
        _function{[this, coro_function]
        {
            coro_function(*this);
        }}
    {
        _context = _pool->make_context(_function);
    }

    std::size_t context() const
    {
        return _context;
    }

    context_pool* pool() const
    {
        return _pool;
    }

    void switch_to(const coroutine& coro)
    {
        assert(_pool == coro.pool());
        _pool->switch_to(coro.context());
    }

    template<typename Value>
    void switch_to(const coroutine& coro, Value&& value)
    {
        _channel.set(std::forward<Value>(value));
        switch_to(coro);
    }

    const T& get() const
    {
        return _channel.get();
    }

    void operator()()
    {
        switch_to(*this);
    }

    template<typename Value>
    void operator()(Value&& value)
    {
        switch_to(*this, std::forward<Value>(value));
    }

    void operator()(const coroutine& coro)
    {
        switch_to(coro);
    }

    template<typename Value>
    void operator()(const coroutine& coro, Value&& value)
    {
        switch_to(coro, std::forward<Value>(value));
    }

    bool alive() const
    {
        auto* context = _pool->context(_context);

        return context != nullptr &&
               context->state != context_pool::context_data::context_state::dead &&
               context->state != context_pool::context_data::context_state::idle;
    }

    operator bool() const
    {
        return alive();
    }

private:
    context_pool* _pool;
    std::size_t _context;
    value_channel<T> _channel;
    coro::stateful_callback<std::function<void()>> _function;
};

}

}

#endif // COROUTINE_BACKEND_COROUTINE_HPP
