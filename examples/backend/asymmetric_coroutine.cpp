#include <coroutine/backend/asymmetric_coroutine.hpp>
#include <iostream>

using pull_t = coro::back::asymmetric_coroutine::pull_type<int>;
using push_t = coro::back::asymmetric_coroutine::push_type<int>;

int main()
{
    pull_t source{[&](push_t& sink)
    {
        for(int i = 0; i < 3; ++i)
        {
            std::cout << "source " << i << "\n";
            sink(i);
        }

        std::cout << "end of source\n";
    }};

    for(int i : source)
    {
        std::cout << "main thread " << i << "\n";
    }

    std::cout << "end of main thread\n";
}
