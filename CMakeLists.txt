cmake_minimum_required(VERSION 3.20)
project(mqlite LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)


# ASIO
FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio
    GIT_TAG        asio-1-38-2
)
FetchContent_MakeAvailable(asio)

add_library(asio_standalone INTERFACE)
target_include_directories(asio_standalone INTERFACE ${asio_SOURCE_DIR}/include)
target_compile_definitions(asio_standalone INTERFACE ASIO_STANDALONE)

# ASIO also required Threads library on most platforms
find_package(Threads REQUIRED)


# mqlite
file(GLOB_RECURSE mqlite_SRCS "src/*.cpp")
add_executable(mqlite ${mqlite_SRCS})
target_link_libraries(mqlite 
PRIVATE
    Threads::Threads
    asio_standalone
)