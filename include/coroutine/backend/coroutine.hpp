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
    const T& set(Value&& value)
    {
        return _value = std::forward<Value>(value);
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

    coroutine() :
        _pool{&get_context_pool()},
        _context{_pool->current_context().id}
    {}

    std::size_t context() const
    {
        return _context;
    }

    context_pool* pool() const
    {
        return _pool;
    }

    void switch_to()
    {
        switch_to(*this);
    }

    coroutine& switch_to(const coroutine& coro)
    {
        assert(_pool == coro.pool());
        _pool->switch_to(coro.context());
        return *this;
    }

    template<typename Value>
    coroutine& switch_to(const coroutine& coro, Value&& value)
    {
        set(std::forward<Value>(value));
        return switch_to(coro);
    }

    coroutine& yield()
    {
        _pool->yield(_context);
        return *this;
    }

    template<typename Value>
    coroutine& yield(Value&& value)
    {
        set(std::forward<Value>(value));
        return yield();
    }

    const T& get() const
    {
        return _channel.get();
    }

    template<typename Value>
    const T& set(Value&& value)
    {
        return _channel.set(std::forward<Value>(value));
    }

    coroutine& operator()()
    {
        return switch_to(*this);
    }

    template<typename Value>
    coroutine& operator()(Value&& value)
    {
        return switch_to(*this, std::forward<Value>(value));
    }

    coroutine& operator()(const coroutine& coro)
    {
        return switch_to(coro);
    }

    template<typename Value>
    coroutine& operator()(const coroutine& coro, Value&& value)
    {
        return switch_to(coro, std::forward<Value>(value));
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

    class iterator
    {
    public:
        iterator(coroutine* self) :
            _self{self}
        {}

        iterator& operator++()
        {
            _self->switch_to();
            return *this;
        }

        iterator operator++(int)
        {
            return operator++();
        }

        friend bool operator==(iterator lhs, iterator rhs)
        {
            return lhs._self == rhs._self && !lhs._self->alive();
        }

        friend bool operator!=(iterator lhs, iterator rhs)
        {
            return !(lhs == rhs);
        }

        const T& operator*() const
        {
            _self->yield();
            return _self->get();
        }

        class value_proxy
        {
        public:
            value_proxy(coroutine* self) :
                _self{self}
            {}

            template<typename Value>
            const T& operator=(Value&& value)
            {
                return _self->set(std::forward<Value>(value));
            }

            operator const T&() const
            {
                return _self->get();
            }

        private:
            coroutine* _self;
        };

        value_proxy operator*()
        {
            return {_self};
        }

    private:
        coroutine* _self;
    };

    iterator begin()
    {
        return {this};
    }

    iterator end()
    {
        return {this};
    }

    static coroutine current()
    {
        return {};
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
