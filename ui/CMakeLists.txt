cmake_minimum_required(VERSION 3.16)
project(ascii_studio LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)
find_package(Python3 COMPONENTS Interpreter REQUIRED)

# The Python conversion scripts and their fonts live one level above ui/.
get_filename_component(ASCII_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)

# Dear ImGui with the SDL2/OpenGL3 backend.
add_library(imgui STATIC
    third_party/imgui/imgui.cpp
    third_party/imgui/imgui_draw.cpp
    third_party/imgui/imgui_tables.cpp
    third_party/imgui/imgui_widgets.cpp
    third_party/imgui/backends/imgui_impl_sdl2.cpp
    third_party/imgui/backends/imgui_impl_opengl3.cpp
    third_party/imgui/misc/cpp/imgui_stdlib.cpp
)
target_include_directories(imgui PUBLIC
    third_party/imgui
    third_party/imgui/backends
    third_party/imgui/misc/cpp
)
target_link_libraries(imgui PUBLIC SDL2::SDL2 OpenGL::GL)

add_executable(ascii_studio
    src/main.cpp
    src/theme.cpp
    src/image.cpp
    src/convert.cpp
    src/file_dialog.cpp
)
target_include_directories(ascii_studio PRIVATE third_party/stb)
target_link_libraries(ascii_studio PRIVATE
    imgui
    SDL2::SDL2
    OpenGL::GL
    Threads::Threads
    ${CMAKE_DL_LIBS}
)
if(TARGET SDL2::SDL2main)
    target_link_libraries(ascii_studio PRIVATE SDL2::SDL2main)
endif()

target_compile_definitions(ascii_studio PRIVATE
    ASCII_PROJECT_ROOT="${ASCII_ROOT}"
    ASCII_PYTHON="${Python3_EXECUTABLE}"
)
