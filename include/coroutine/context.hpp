#ifndef COROUTINE_CONTEXT_HPP
#define COROUTINE_CONTEXT_HPP

namespace coro
{

class Context
{
public:
    struct ContextDeleter
    {
        void operator()(void* contextHandle);
    };

    /**
     * \brief Returns the currently active context
     */
    static Context get();

    

private:
    Context(void* contextHandle);

    using Handle = std::unique_ptr<void, ContextDeleter>;

    Handle _contextHandle;
};

}

#endif // COROUTINE_CONTEXT_HPP
