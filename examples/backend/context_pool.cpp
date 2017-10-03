#include <coroutine/backend/context_pool.hpp>
#include <iostream>

void foo(int n);
void bar(int n);

auto foo_call = coro::make_stateful_callback([]
{
    foo(42);
});
auto bar_call = coro::make_stateful_callback([]
{
    bar(42);
});

std::size_t foo_id = 0;
std::size_t bar_id = 0;

void foo(int n)
{
    for(int i = 0; i < n; ++i)
    {
        std::cout << "foo " << i << "\n";
        coro::back::get_context_pool().switch_to(bar_id);
    }

    std::cout << "end foo\n";
}

void bar(int n)
{
    for(int i = 0; i < n; ++i)
    {
        std::cout << "bar " << i << "\n";
        coro::back::get_context_pool().yield();
    }

    std::cout << "end bar\n";
}

int main()
{
    foo_id = coro::back::get_context_pool().make_context(foo_call, "foo");
    bar_id = coro::back::get_context_pool().make_context(bar_call, "bar");
    coro::back::get_context_pool().switch_to(foo_id);

    std::cout << "end of main thread\n";
}
