#include <coroutine/sized_memory_block.hpp>
#include <cstdlib>

namespace coro
{

sized_memory_block malloc(std::size_t bytes)
{
    sized_memory_block result{};

    if((result.start = std::malloc(bytes)))
    {
        result.size = bytes;
    }

    return result;
}

void free(const sized_memory_block& block)
{
    std::free(block.start);
}

}
