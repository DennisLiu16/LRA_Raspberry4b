# The first target makefile of LRA project

cmake_minimum_required(VERSION 3.12)
project(LRA)

# global var
## debug usage ##
set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CMAKE_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/inc)

# add_sub_directories , remember add CMakeList.txt
add_subdirectory(${PROJECT_SOURCE_DIR}/src)


