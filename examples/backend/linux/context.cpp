#include <coroutine/backend/linux/context.hpp>
#include <iostream>

void foo(int n);
void bar(int n);

coro::back::context main_ctx, foo_ctx, bar_ctx;

char foo_stack[1024];
char bar_stack[1024];

auto foo_call = coro::make_stateful_callback([]
{
    foo(42);
});
auto bar_call = coro::make_stateful_callback([]
{
    bar(42);
});

void foo(int n)
{
    for(int i = 0; i < n; ++i)
    {
        std::cout << "foo " << i << "\n";
        coro::back::swap_context(foo_ctx, bar_ctx);
    }

    std::cout << "end foo\n";
}

void bar(int n)
{
    for(int i = 0; i < n; ++i)
    {
        std::cout << "bar " << i << "\n";
        coro::back::swap_context(bar_ctx, foo_ctx);
    }

    std::cout << "end bar\n";
}

int main()
{
    main_ctx = coro::back::get_current_context();
    foo_ctx = coro::back::make_context(foo_call, coro::static_block(foo_stack), main_ctx);
    bar_ctx = coro::back::make_context(bar_call, coro::static_block(bar_stack), main_ctx);

    coro::back::swap_context(main_ctx, foo_ctx);
}
