add_executable(examples-backend-context context.cpp)
target_link_libraries(examples-backend-context PRIVATE coroutine)

add_executable(examples-backend-context-pool context_pool.cpp)
target_link_libraries(examples-backend-context-pool PRIVATE coroutine)

add_executable(examples-backend-coroutine coroutine.cpp)
target_link_libraries(examples-backend-coroutine PRIVATE coroutine)

add_executable(examples-backend-asymmetric_coroutine asymmetric_coroutine.cpp)
target_link_libraries(examples-backend-asymmetric_coroutine PRIVATE coroutine)
