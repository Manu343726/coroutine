#ifndef COROUTINE_BACKEND_CONTEXT_POOL_HPP
#define COROUTINE_BACKEND_CONTEXT_POOL_HPP

#include <coroutine/backend/context.hpp>
#include <unordered_map>
#include <memory>

namespace coro
{

namespace back
{

class context_pool
{
public:
    struct context_data
    {
        std::size_t return_id;
        std::size_t caller_id;
        context_pool* pool;
        const coro::callback* callback;
        coro::back::context last_context;
        coro::sized_memory_block stack;

        std::size_t id() const
        {
            return coro::back::context_id(last_context);
        }
        bool unused() const
        {
            return pool == nullptr;
        }
    };

    context_pool();

    std::size_t make_context(const callback& callback);
    void switch_to(std::size_t context);
    void yield();

    std::size_t main_context() const;
    context_data& current_context();

private:
    context_data _pool[128];
    std::size_t _main_context_id = 0;
    coro::callback _pool_runner;

    std::size_t current_context_id() const;
    void remove_context(std::size_t id);
    void release_context(context_data& context);
    static void pool_function(void* pool);
    context_data* find_context(std::size_t id);
    context_data* find_unused_context();
};

context_pool& get_context_pool();
const context_pool::context_data& get_context_data();

}

}

#endif // COROUTINE_BACKEND_CONTEXT_POOL_HPP
