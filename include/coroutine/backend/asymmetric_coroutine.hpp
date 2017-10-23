#ifndef COROUTINE_BACKEND_ASYMMETRIC_COROUTINE_HPP
#define COROUTINE_BACKEND_ASYMMETRIC_COROUTINE_HPP

#include <coroutine/backend/coroutine.hpp>
#include <coroutine/backend/detail/aligned_union.hpp>

namespace coro
{

namespace back
{

namespace asymmetric_coroutine
{

template<typename T>
class push_type;

template<typename T>
class coroutine_proxy
{
public:
    enum class proxy_mode
    {
        instance,
        reference
    } mode;

    using instance_t = coro::back::coroutine<T>;
    using reference_t = coro::back::coroutine<T>*;

    using storage_t = typename coro::back::detail::aligned_union<
        0,
        reference_t,
        instance_t
    >::type;

    template<typename Function>
    coroutine_proxy(Function&& function) :
        mode{proxy_mode::instance}
    {
        new(&storage) instance_t{std::forward<Function>(function)};
    }

    coroutine_proxy(coro::back::coroutine<T>* reference) :
        mode{proxy_mode::reference}
    {
        new (&storage) reference_t{reference};
    }

    coroutine_proxy(const coroutine_proxy&) = delete;
    coroutine_proxy(coroutine_proxy&&) = delete;
    coroutine_proxy& operator=(const coroutine_proxy&) = delete;
    coroutine_proxy& operator=(coroutine_proxy&&) = delete;

    ~coroutine_proxy()
    {
        if(mode == proxy_mode::instance)
        {
            get_coro()->~instance_t();
        }
    }

    coro::back::coroutine<T>* get_coro()
    {
        switch(mode)
        {
        case proxy_mode::instance:
            return reinterpret_cast<instance_t*>(&storage);
        case proxy_mode::reference:
            return *reinterpret_cast<reference_t*>(&storage);
        }
    }

    const coro::back::coroutine<T>* get_coro() const
    {
        switch(mode)
        {
        case proxy_mode::instance:
            return reinterpret_cast<const instance_t*>(&storage);
        case proxy_mode::reference:
            return *reinterpret_cast<const reference_t*>(&storage);
        }
    }

    storage_t storage;
};

template<typename T>
class pull_type
{
    using proxy = coro::back::asymmetric_coroutine::coroutine_proxy<T>;
    using coroutine = coro::back::coroutine<T>;
    using push_type = coro::back::asymmetric_coroutine::push_type<T>;

    friend push_type;

public:
    template<typename Function>
    pull_type(Function coro_function) :
        _proxy{[this, coro_function](coroutine& coro)
        {
            push_type push{this};
            coro_function(push);
        }}
    {}

private:
    pull_type(push_type* push) :
        _proxy{push->_proxy.get_coro()}
    {}

public:
    bool alive() const
    {
        return _proxy.get_coro()->alive();
    }

    operator bool() const
    {
        return alive();
    }

    const T& get() const
    {
        return _proxy.get_coro()->get();
    }

    pull_type& operator()()
    {
        if(alive())
        {
            _proxy.get_coro()->asymmetric_switch();
        }
        return *this;
    }

    pull_type& yield()
    {
        if(alive())
        {
            _proxy.get_coro()->asymmetric_yield();
        }
        return *this;
    }

    class iterator
    {
    private:
        void fetch()
        {
            _value = _self->get();
        }

        void increment()
        {
            (*_self)();
            fetch();
        }

    public:
        iterator() :
            _self{nullptr}
        {}

        iterator(pull_type* self) :
          _self{self}
        {
            increment();
        }

        iterator& operator++()
        {
            increment();
            return *this;
        }

        const T& operator*() const
        {
            return _value;
        }

        friend bool operator==(const iterator& lhs, const iterator& rhs)
        {
            return lhs._self != nullptr && !lhs._self->alive();
        }

        friend bool operator!=(const iterator& lhs, const iterator& rhs)
        {
            return !(lhs == rhs);
        }

    private:
        pull_type* _self;
        T _value;
    };

    using const_iterator = iterator;

    iterator begin()
    {
        return {this};
    }

    iterator end()
    {
        return {};
    }

    const_iterator begin() const
    {
        return {this};
    }

    const_iterator end() const
    {
        return {};
    }

private:
    proxy _proxy;
};

template<typename T>
class push_type
{
    using proxy = coro::back::asymmetric_coroutine::coroutine_proxy<T>;
    using coroutine = coro::back::coroutine<T>;
    using pull_type = coro::back::asymmetric_coroutine::pull_type<T>;

    friend pull_type;

public:
    template<typename Function>
    push_type(Function coro_function) :
        _proxy{[this, coro_function](coroutine& coro)
        {
            pull_type pull{this};
            coro_function(pull);
        }}
    {}

private:
    push_type(pull_type* pull) :
        _proxy{pull->_proxy.get_coro()}
    {}

public:
    bool alive() const
    {
        return _proxy.get_coro()->alive();
    }

    operator bool() const
    {
        return alive();
    }

    template<typename Value>
    push_type& set(Value&& value)
    {
        _proxy.get_coro()->set(std::forward<Value>(value));
        return *this;
    }

    template<typename Value>
    push_type& operator()(Value&& value)
    {
        return yield(std::forward<Value>(value));
    }

    push_type& operator()()
    {
        return yield();
    }

    push_type& yield()
    {
        _proxy.get_coro()->assymetric_yield();
        return *this;
    }

    template<typename Value>
    push_type& yield(Value&& value)
    {
        set(std::forward<Value>(value));
        return yield();
    }

    class iterator
    {
    public:
        iterator() :
            _self{nullptr}
        {}

        iterator(push_type* self) :
            _self{self}
        {}

        iterator& operator++()
        {
            return *this;
        }

        iterator& operator*()
        {
            return *this;
        }

        template<typename Value>
        iterator& operator=(Value&& value)
        {
            (*_self)(std::forward<Value>(value));
            return *this;
        }

        friend bool operator==(const iterator& lhs, const iterator& rhs)
        {
            return lhs._self == rhs._self && lhs._self != nullptr && !lhs._self->alive();
        }

        friend bool operator!=(const iterator& lhs, const iterator& rhs)
        {
            return !(lhs == rhs);
        }

    private:
        push_type* _self;
    };

    using const_iterator = iterator;

    iterator begin()
    {
        return {this};
    }

    iterator end()
    {
        return {};
    }

    const_iterator begin() const
    {
        return {this};
    }

    const_iterator end() const
    {
        return {};
    }

private:
    proxy _proxy;
};

}

}

}

#endif // COROUTINE_BACKEND_ASYMMETRIC_COROUTINE_HPP
