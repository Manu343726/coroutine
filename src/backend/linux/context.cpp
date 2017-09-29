#include <coroutine/backend/linux/context.hpp>
#include <cstdint>
#include <cstdarg>

static_assert(std::is_same<std::int32_t, int>::value, "int must be 32 bits wide");

template<typename T>
int pointer_high(const T* ptr)
{
    return reinterpret_cast<std::uintptr_t>(const_cast<T*>(ptr)) >> 32;
}

template<typename T>
int pointer_low(const T* ptr)
{
    return static_cast<int>(reinterpret_cast<std::uintptr_t>(const_cast<T*>(ptr)) & 0xFFFFFFFF);
}

template<typename T>
const T* pointer(int high, int low)
{
    return reinterpret_cast<const T*>(
        (static_cast<std::uintptr_t>(high) << 32) | (static_cast<std::uintptr_t>(low) & 0xFFFFFFFF)
    );
}

extern "C"
{

void coroutine_context_func_64(int high, int low)
{
    const auto* callback = pointer<coro::callback>(high, low);
    callback->function(callback->context);
}

void coroutine_context_func_32(int callback_ptr)
{
    const auto* callback = reinterpret_cast<coro::callback*>(
        callback_ptr
    );

    callback->function(callback->context);
}

}

namespace coro
{

namespace back
{

context get_current_context()
{
    context result{};
    ::getcontext(&result);
    return result;
}

void swap_context(context& from, const context& to)
{
    ::swapcontext(&from, &to);
}

context make_context(const callback& callback, const sized_memory_block& stack, const context& return_context)
{
    context result = get_current_context();

    result.uc_stack.ss_sp = stack.start;
    result.uc_stack.ss_size = stack.size;
    result.uc_link = const_cast<context*>(&return_context);

#if INTPTR_MAX == INT32_MAX
    ::makecontext(
        &result,
        reinterpret_cast<void(*)()>(::coroutine_context_func_32),
        1,
        reinterpret_cast<std::uintptr_t>(const_cast<coro::callback*>(&callback))
    );
#elif INTPTR_MAX == INT64_MAX
    ::makecontext(
        &result,
        reinterpret_cast<void(*)()>(::coroutine_context_func_64),
        2,
        pointer_high(&callback), pointer_low(&callback)
    );
#endif // INTPTR_MAX == INTXX_MAX

    return result;
}

std::size_t context_id(const context& context)
{
    return reinterpret_cast<std::size_t>(context.uc_stack.ss_sp);
}

coro::sized_memory_block allocate_stack(std::size_t bytes)
{
    return coro::malloc(SIGSTKSZ*2);
}

void free_stack(sized_memory_block& stack)
{
    return coro::free(stack);
}

}

}
