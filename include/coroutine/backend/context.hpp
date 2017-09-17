#ifndef COROUTINE_BACKEND_CONTEXT_HPP
#define COROUTINE_BACKEND_CONTEXT_HPP

#ifdef _WIN32
#include <coroutine/backend/windows/context.hpp>
#endif // _WIN32

#ifdef linux
#include <coroutine/backend/linux/context.hpp>
#endif // linux

#endif // COROUTINE_BACKEND_CONTEXT_HPP