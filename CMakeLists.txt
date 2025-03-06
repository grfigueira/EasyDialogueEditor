cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 20)
project(EasyDialogueEditor)

find_package(OpenGL REQUIRED)

add_subdirectory(vendors)
add_subdirectory(src)