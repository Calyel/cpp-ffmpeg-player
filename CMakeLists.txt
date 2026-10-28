cmake_minimum_required(VERSION 3.16)

project(
    cpp-ffmpeg-player
    VERSION 1.0.0
    DESCRIPTION "Ferramenta CLI e biblioteca C++ para inspecao de metadados (ffprobe) e reproducao de video (ffplay)"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Biblioteca core reutilizavel
add_library(ffmpeg_player_core STATIC
    src/ProcessRunner.cpp
    src/MediaInfo.cpp
    src/VideoPlayer.cpp
)

target_include_directories(ffmpeg_player_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Executavel principal
add_executable(cpp-ffmpeg-player src/main.cpp)
target_link_libraries(cpp-ffmpeg-player PRIVATE ffmpeg_player_core)

# Testes automatizados
enable_testing()
add_executable(test_runner tests/test_main.cpp)
target_link_libraries(test_runner PRIVATE ffmpeg_player_core)

add_test(
    NAME unit_and_integration_tests
    COMMAND test_runner
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
