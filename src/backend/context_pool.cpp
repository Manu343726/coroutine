#include <coroutine/backend/context_pool.hpp>

using namespace coro::back;

void context_pool::pool_function(void* data)
{
    auto* pool = reinterpret_cast<context_pool*>(data);
    auto* context = pool->_current_context;

    if(context->callback != nullptr)
    {
        context->callback->invoke();
    }

    context->state = context_pool::context_data::context_state::dead;

    auto* caller_context = context->caller_context;
    auto* return_context = context->return_context;

    if(caller_context != nullptr)
    {
        pool->switch_to(caller_context);
    }
    else if(return_context != nullptr)
    {
        pool->switch_to(return_context);
    }

    pool->switch_to(pool->_main_context);
}

context_pool::context_pool() :
    _pool{}
{
    for(std::size_t i = 0; i < 128; ++i)
    {
        _pool[i].stack = coro::back::allocate_stack(coro::back::stack_size());
        _pool[i].id = i;
        _pool[i].description = "dead";
        _pool[i].state = context_pool::context_data::context_state::idle;
    }

    _pool_runner.function = &context_pool::pool_function;
    _pool_runner.context = this;

    auto main_context = coro::back::get_current_context();
    auto* main_context_data = find_unused_context();
    main_context_data->last_context = main_context;
    main_context_data->caller_context = nullptr;
    main_context_data->return_context = nullptr;
    main_context_data->pool = this;
    main_context_data->callback = nullptr;
    main_context_data->description = "main context";
    main_context_data->state = context_pool::context_data::context_state::active;
    _main_context = main_context_data;
    _current_context = _main_context;
}

context_pool::~context_pool()
{
    for(auto& context : _pool)
    {
        coro::back::free_stack(context.stack);
    }
}

std::size_t context_pool::make_context(const coro::callback& callback, const char* description)
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
    context_data->return_context = _current_context;
    context_data->caller_context = _current_context;
    context_data->pool = this;
    context_data->callback = &callback;
    context_data->description = description;
    context_data->state = context_pool::context_data::context_state::ready;
    return context_data->id;
}

void context_pool::switch_to(std::size_t context_id)
{
    switch_to(find_context(context_id));
}

void context_pool::switch_to(context_pool::context_data* context)
{
    if(context != nullptr && context->state == context_pool::context_data::context_state::ready &&
       context != _current_context)
    {
        auto& caller = _current_context->last_context;
        auto* old_caller_context = context->caller_context;
        auto* old_current_context = _current_context;

        context->caller_context = _current_context;

        if(_current_context->state == context_pool::context_data::context_state::active)
        {
            _current_context->state = context_pool::context_data::context_state::ready;
        }
        _current_context = context;
        context->state = context_pool::context_data::context_state::active;
        coro::back::swap_context(caller, context->last_context);
        context->caller_context = old_caller_context;

        if (context->state == context_pool::context_data::context_state::active)
        {
            context->state = context_pool::context_data::context_state::ready;
        }
        else if (context->state == context_pool::context_data::context_state::dead)
        {
            // Dead contexts are released here to make sure we never release an active
            // context. This is not critical with ucontext since coro::back::release_context()
            // is basically a noop there, but we have to be careful when releasing Win32 fibers.
            release_context(*context);
        }
    }
}

void context_pool::yield()
{
    switch_to(current_context().caller_context);
}

const context_pool::context_data& context_pool::current_context() const
{
    return *_current_context;
}

const context_pool::context_data& context_pool::main_context() const
{
    return *_main_context;
}

const context_pool::context_data* context_pool::context(std::size_t id) const
{
    return find_context(id);
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
    context.caller_context = nullptr;
    context.return_context = nullptr;
    context.description = "dead";
    context.state = context_pool::context_data::context_state::idle;
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
        if(!context.unused() && context.id == id)
        {
            return &context;
        }
    }

    return nullptr;
}

const context_pool::context_data* context_pool::find_context(std::size_t id) const
{
    for(auto& context : _pool)
    {
        if(!context.unused() && context.id == id)
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
