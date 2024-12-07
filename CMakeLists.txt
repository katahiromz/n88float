# CMakeLists.txt --- CMake project settings
##############################################################################

# CMake minimum version
cmake_minimum_required(VERSION 3.6)

# project name and languages
project(n88float CXX)

# n88float.exe
add_executable(n88float n88float.cpp)
target_compile_definitions(n88float PRIVATE N88FLOAT_EXE)

##############################################################################
