#include <coroutine/backend/windows/context.hpp>

extern "C"
{
    VOID __stdcall coroutine_fiber_func(LPVOID fiberParam)
    {
        const auto* callback = reinterpret_cast<coro::callback*>(fiberParam);
        callback->function(callback->context);
    }
}

namespace coro
{

namespace back
{
context get_current_context()
{
    thread_local LPVOID thread_fiber = ::ConvertThreadToFiber(nullptr);
    auto fiber = ::GetCurrentFiber();

    if(fiber != nullptr)
    {
        return {fiber, ::GetCurrentThread()};
    }
    else
    {
        return {thread_fiber, ::GetCurrentThread()};
    }
}

void swap_context(context&, const context& to)
{
    ::SwitchToFiber(to.handle);
}

context make_context(const callback& callback, const sized_memory_block& stack, const context& return_context)
{
    coro::back::context context;
    context.handle = ::CreateFiber(0, &::coroutine_fiber_func, const_cast<coro::callback*>(&callback));
    context.thread = ::GetCurrentThread();
    context.next_context = &return_context;

    return context;
}

coro::sized_memory_block allocate_stack(std::size_t bytes)
{
    return { nullptr, 0 };
}

void free_stack(const coro::sized_memory_block& stack)
{}

void release_context(const context& context)
{
    ::DeleteFiber(context.handle);
}

}

}