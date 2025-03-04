set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(EasyDialogEditor 
    main.cpp
    Node.h
    easy_dialog_editor.cpp
    show_windows.h
    show_windows.cpp
    WindowsPlatformUtils.cpp
    resources/resource.rc
)

target_link_libraries(EasyDialogEditor
    PRIVATE
	SDL2::SDL2main
    SDL2::SDL2
    OpenGL::GL
    imgui
    imnodes
)

target_compile_definitions(EasyDialogEditor PRIVATE SDL_STATIC)

# Update the post-build command to copy SDL2 DLL instead
#add_custom_command(TARGET EasyDialogEditor POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#    "${CMAKE_BINARY_DIR}/_deps/sdl2-build/SDL2.dll"
#    "$<TARGET_FILE_DIR:EasyDialogEditor>"
#)