# The first target makefile of LRA project

cmake_minimum_required(VERSION 3.12)
project(LRA)

# global var
set(CMAKE_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/inc)
## debug usage ##
set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")
set(CMAKE_C_FLAGS_DEBUG "-g -DDEBUG")



# add_sub_directories , remember add CMakeList.txt
add_subdirectory(${PROJECT_SOURCE_DIR}/src)


