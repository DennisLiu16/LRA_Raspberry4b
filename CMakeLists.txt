# The first target makefile of LRA project

cmake_minimum_required(VERSION 3.12)
project(LRA)

## recursive var
# recursive driver setting
file(GLOB_RECURSE SRC_DRV2605L_SETTING ${CMAKE_CURRENT_SOURCE_DIR}/src/drv2605L/setting/*.cpp)

## include path
set(CMAKE_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/inc/)

message("====================================================================================")

# You should create src/lib/LRA_LibConfig.cmake first and add_library there to create .so file
# option(BUILD_SHARED_LIBS "Build using shared libraries" ON)
add_library(LRA_Lib SHARED)


## compile and link configure
# an example for Driver Setting
add_executable(LRA_Driver_Setting)
target_sources(LRA_Driver_Setting PRIVATE ${SRC_DRV2605L_SETTING})
target_link_libraries(LRA_Driver_Setting LRA_Lib)
# target_include_directories(LRA_Driver_Setting PRIVATE ${CMAKE_INCLUDE_PATH})
target_compile_definitions(LRA_Driver_Setting PRIVATE std_cxx_17)
#target_compile_options()

# https://cmake.org/cmake/help/latest/guide/tutorial/Selecting%20Static%20or%20Shared%20Libraries.html

## recursive var
# recursive lib files
file(GLOB_RECURSE SRC_LIB ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp) 

# build .so file
add_library(LRA_Lib SHARED)
