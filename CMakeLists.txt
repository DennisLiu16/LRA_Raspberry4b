# The first target makefile of LRA project

cmake_minimum_required(VERSION 3.16...3.22)
if(${CMAKE_VERSION} VERSION_LESS 3.18)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(LRA LANGUAGES C CXX
            VERSION 1.0
            DESCRIPTION "A project for LRA control and show data"
        )

# global var
set(CMAKE_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/inc)
## debug usage ##
set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG -O3 -ftime-report")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")  # O3
set(CMAKE_C_FLAGS_DEBUG "-g -DDEBUG")



# add_sub_directories , remember add CMakeList.txt
add_subdirectory(${PROJECT_SOURCE_DIR}/src)


