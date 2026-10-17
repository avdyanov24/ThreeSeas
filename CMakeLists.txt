cmake_minimum_required(VERSION 3.20)
project(crossroads CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Werror)
endif()

include(FetchContent)
FetchContent_Declare(raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 6.0)
FetchContent_MakeAvailable(raylib)

# Game logic: no raylib, so tests can use it without a window
add_library(logic src/maze.cpp src/game.cpp src/levels.cpp
                  src/validation.cpp src/save.cpp)
target_include_directories(logic PUBLIC src)

add_executable(crossroads src/main.cpp src/render.cpp
                          src/screens.cpp src/audio.cpp)
target_link_libraries(crossroads logic raylib)

enable_testing()
add_executable(tests tests/tests.cpp)
target_link_libraries(tests logic)
add_test(NAME tests COMMAND tests)
