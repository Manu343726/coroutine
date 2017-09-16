#ifndef COROUTINE_CALLBACK_HPP
#define COROUTINE_CALLBACK_HPP

#include <utility>

namespace coro
{

struct callback
{
    void(*function)(void*);
    void* context;
};

template<typename Function>
struct stateful_callback : public callback
{
    stateful_callback(Function function) :
        stateful_function{std::move(function)}
    {
        callback::function = +[](void* context)
        {
            auto& stateful_function = *reinterpret_cast<Function*>(context);
            stateful_function();
        };
        callback::context = &stateful_function;
    }

    Function stateful_function;
};

template<typename Function>
stateful_callback<Function> make_stateful_callback(Function function)
{
    return {std::move(function)};
}

}

#endif // COROUTINE_CALLBACK_HPP
