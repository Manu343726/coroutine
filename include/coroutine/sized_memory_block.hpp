#ifndef COROUTINE_SIZED_MEMORY_BLOCK_HPP
#define COROUTINE_SIZED_MEMORY_BLOCK_HPP

#include <cstdint>

namespace coro
{

struct sized_memory_block
{
    void* start;
    std::size_t size;
    void* end() const;
};

sized_memory_block malloc(std::size_t bytes);
sized_memory_block aligned_malloc(std::size_t bytes, std::size_t alignment);
void free(const sized_memory_block& block);
void aligned_free(const sized_memory_block& block);

template<std::size_t N>
sized_memory_block static_block(const char (&block)[N])
{
    return {reinterpret_cast<void*>(const_cast<char*>(&block[0])), N};
}

}

#endif // COROUTINE_SIZED_MEMORY_BLOCK_HPP
