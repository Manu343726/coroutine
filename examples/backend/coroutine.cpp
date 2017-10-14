#include <coroutine/backend/coroutine.hpp>
#include <iostream>

using coro_t = coro::back::coroutine<int>;

int main()
{
    coro_t* a_ref;
    coro_t* b_ref;

    coro_t a{[&](coro_t& coro)
    {
        auto n = coro.get();
        for(int i = 0; i < n; ++i)
        {
            std::cout << "a " << i << "\n";
            coro.switch_to(*b_ref, i);
        }

        std::cout << "end of a\n";
    }};

    coro_t b{[&](coro_t& coro)
    {
        while(*a_ref)
        {
            std::cout << "b " << a_ref->get() << "\n";
            coro.switch_to(*a_ref);
        }

        std::cout << "end of b\n";
    }};

    a_ref = &a;
    b_ref = &b;

    a(42);

    std::cout << "end of main thread\n";
}
