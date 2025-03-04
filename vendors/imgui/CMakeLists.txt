include(FetchContent)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        v1.90.2-docking
)

FetchContent_MakeAvailable(imgui)

add_library(imgui)

target_include_directories(imgui
    PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${imgui_SOURCE_DIR}/misc/cpp
)

target_sources(
    imgui
    PUBLIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Instead of using find_package(SDL2), link directly to the SDL2-static target
# that was created when SDL2 was built with FetchContent
target_link_libraries(imgui PUBLIC SDL2-static OpenGL::GL)

# Define SDL_STATIC for all code that uses imgui
target_compile_definitions(imgui PUBLIC SDL_STATIC)

set(imgui_SOURCE_DIR ${imgui_SOURCE_DIR} PARENT_SCOPE)