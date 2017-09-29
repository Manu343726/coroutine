#include <coroutine/backend/context_pool.hpp>

using namespace coro::back;

using stack = char[coro::back::stack_size()];

coro::sized_memory_block get_stack(std::size_t i)
{
    static stack stack[128];
    return coro::static_block(stack[i]);
}

void context_pool::pool_function(void* data)
{
    auto* pool = reinterpret_cast<context_pool*>(data);
    const auto& context = pool->current_context();

    if(context.callback != nullptr)
    {
        context.callback->invoke();
    }

    auto return_id = context.return_id;
    pool->remove_context(context.id());

    if(return_id != 0)
    {
        pool->switch_to(return_id);
    }
}

context_pool::context_pool() :
    _pool{}
{
    for(std::size_t i = 0; i < 128; ++i)
    {
        _pool[i].stack = get_stack(i);
    }

    _pool_runner.function = &context_pool::pool_function;
    _pool_runner.context = this;

    auto main_context = coro::back::get_current_context();
    _main_context_id = coro::back::context_id(main_context);
    auto* main_context_data = find_unused_context();
    main_context_data->last_context = main_context;
    main_context_data->caller_id = 0;
    main_context_data->return_id = 0;
    main_context_data->pool = this;
    main_context_data->callback = nullptr;
}

std::size_t context_pool::make_context(const coro::callback& callback)
{
    auto* context_data = find_unused_context();

    if(context_data == nullptr)
    {
        return 0;
    }

    context_data->last_context = coro::back::make_context(
        _pool_runner,
        context_data->stack,
        current_context().last_context
    );
    context_data->return_id = current_context_id();
    context_data->caller_id = context_data->return_id;
    context_data->pool = this;
    context_data->callback = &callback;
    return context_data->id();
}

void context_pool::switch_to(std::size_t context_id)
{
    auto* context = find_context(context_id);

    if(context != nullptr)
    {
        auto& caller = current_context().last_context;
        auto caller_id = context->caller_id;
        context->caller_id = current_context_id();
        coro::back::swap_context(caller, context->last_context);
        context->caller_id = caller_id;
    }
}

void context_pool::yield()
{
    switch_to(current_context().caller_id);
}

std::size_t context_pool::current_context_id() const
{
    return coro::back::context_id(coro::back::get_current_context());
}

context_pool::context_data& context_pool::current_context()
{
    return *find_context(current_context_id());
}

std::size_t context_pool::main_context() const
{
    return _main_context_id;
}

void context_pool::remove_context(std::size_t id)
{
    auto* context = find_context(id);

    if(context != nullptr)
    {
        release_context(*context);
    }
}

void context_pool::release_context(context_pool::context_data& context)
{
    coro::back::release_context(context.last_context);
    context.pool = nullptr;
}

context_pool::context_data* context_pool::find_unused_context()
{
    for(auto& context : _pool)
    {
        if(context.unused())
        {
            return &context;
        }
    }

    return nullptr;
}

context_pool::context_data* context_pool::find_context(std::size_t id)
{
    for(auto& context : _pool)
    {
        if(!context.unused() && context.id() == id)
        {
            return &context;
        }
    }

    return nullptr;
}

namespace coro
{

namespace back
{

context_pool& get_context_pool()
{
    thread_local context_pool pool;
    return pool;
}

const context_pool::context_data& get_context_data()
{
    return get_context_pool().current_context();
}

}

}
