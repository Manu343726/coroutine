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
        enum class context_state
        {
            idle, ready, active, io_wait, dead
        };
        context_data* caller_context;
        context_data* return_context;
        context_pool* pool;
        const coro::callback* callback;
        coro::back::context last_context;
        coro::sized_memory_block stack;
        const char* description;
        std::size_t id;
        context_state state;

        bool unused() const
        {
            return pool == nullptr;
        }
    };

    context_pool();
    ~context_pool();

    std::size_t make_context(const callback& callback, const char* description = "");
    void switch_to(std::size_t context);
    void yield();

    const context_data& main_context() const;
    const context_data& current_context() const;

private:
    context_data _pool[128];
    context_data* _main_context;
    context_data* _current_context;
    coro::callback _pool_runner;

    void remove_context(std::size_t id);
    void release_context(context_data& context);
    static void pool_function(void* pool);
    context_data* find_context(std::size_t id);
    context_data* find_unused_context();
    void switch_to(context_pool::context_data* context);
};

context_pool& get_context_pool();
const context_pool::context_data& get_context_data();

}

}

#endif // COROUTINE_BACKEND_CONTEXT_POOL_HPP
