#ifndef COROUTINE_BACKEND_WINDOWS_CONTEXT_HPP
#define COROUTINE_BACKEND_WINDOWS_CONTEXT_HPP

#define NOMINMAX
#include <windows.h>
#include <thread>
#include <coroutine/callback.hpp>
#include <coroutine/sized_memory_block.hpp>

namespace coro
{

namespace back
{
    struct context
    {
        LPVOID handle;
        HANDLE thread;
        const context* next_context;
    };

    context get_current_context();
    void swap_context(context& from, const context& to);
    context make_context(const callback& callback, const sized_memory_block& stack, const context& return_context);
}

}

#endif // COROUTINE_BACKEND_WINDOWS_CONTEXT_HPP