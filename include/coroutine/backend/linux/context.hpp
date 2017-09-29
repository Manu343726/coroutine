#ifndef COROUTINE_BACKEND_LINUX_CONTEXT_HPP
#define COROUTINE_BACKEND_LINUX_CONTEXT_HPP

#include <ucontext.h>
#include <coroutine/callback.hpp>
#include <coroutine/sized_memory_block.hpp>

namespace coro
{

namespace back
{

using context = ::ucontext_t;

context get_current_context();
void swap_context(context& from, const context& to);
context make_context(const callback& callback, const sized_memory_block& stack, const context& return_context);
std::size_t context_id(const context& context);
inline void release_context(const context& context) {}
constexpr std::size_t stack_size()
{
    return SIGSTKSZ*2;
}

}

}

#endif // COROUTINE_BACKEND_LINUX_CONTEXT_HPPM#M#
