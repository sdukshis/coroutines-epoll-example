cmake_minimum_required(VERSION 3.10)

project(otus-demo-day CXX)

set(CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_executable(sync_server sync_server.cpp)
add_executable(fork_server fork_server.cpp)
add_executable(thread_server thread_server.cpp)

target_link_libraries(thread_server Threads::Threads)

add_executable(poll_server poll_server.cpp)
add_executable(epoll_server epoll_server.cpp)

add_executable(coro_ex1 coro_ex1.cpp)
target_compile_options(coro_ex1 PUBLIC "-std=c++20")

add_executable(coro_future coro_future.cpp)
target_compile_options(coro_future PUBLIC "-std=c++20")
target_link_libraries(coro_future Threads::Threads)
set_target_properties(coro_future PROPERTIES EXCLUDE_FROM_ALL TRUE)

add_executable(coro_generator coro_generator.cpp)
target_compile_options(coro_generator PUBLIC "-std=c++20")

add_executable(async_server async_server.cpp)
set_target_properties(async_server PROPERTIES EXCLUDE_FROM_ALL TRUE)

target_include_directories(async_server PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/coro-async/coro-async/include")
target_compile_options(async_server PUBLIC "-std=c++20")
target_link_libraries(async_server Threads::Threads)

add_executable(coro_server coro_server.cpp)
set_target_properties(coro_server PROPERTIES EXCLUDE_FROM_ALL TRUE)

target_include_directories(coro_server PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/coro-async/coro-async/include")
target_compile_options(coro_server PUBLIC "-std=c++20")
target_link_libraries(coro_server Threads::Threads)
