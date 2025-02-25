add_library(imnodes STATIC
    imnodes.cpp
)

# Let anyone using `imnodes` see the includes
target_include_directories(imnodes PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"   # So that #include "imnodes.h" works
)

# If you keep ImNodes as a separate library, link it to ImGui:
target_link_libraries(imnodes PUBLIC imgui)
