set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(EasyDialogueEditor 
    main.cpp
    Node.h
	Utils.h
    easy_dialog_editor.cpp
    show_windows.h
    show_windows.cpp
    WindowsPlatformUtils.cpp
    resources/resource.rc
	RobotoFont.hpp
)

target_link_libraries(EasyDialogueEditor
    PRIVATE
	SDL2::SDL2main
    SDL2::SDL2
    OpenGL::GL
    imgui
    imnodes
)

target_compile_definitions(EasyDialogueEditor PRIVATE SDL_STATIC)

# Update the post-build command to copy SDL2 DLL instead
#add_custom_command(TARGET EasyDialogEditor POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#    "${CMAKE_BINARY_DIR}/_deps/sdl2-build/SDL2.dll"
#    "$<TARGET_FILE_DIR:EasyDialogEditor>"
#)