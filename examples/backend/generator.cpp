#include <coroutine/backend/coroutine.hpp>
#include <iostream>

using coro_t = coro::back::coroutine<int>;

int main()
{
    coro_t generator{[&](coro_t& coro)
    {
        auto n = coro.get();

        for(int i = 0; i < n; ++i)
        {
            std::cout << "generator " << i << "\n";
            coro.yield(i);
        }

        std::cout << "end of generator\n";
    }};

    for(int i : generator(42))
    {
        std::cout << "main thread " << i << "\n";
    }

    std::cout << "end of main thread\n";
}
