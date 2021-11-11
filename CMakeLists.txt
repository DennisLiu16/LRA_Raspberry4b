# The first target makefile of LRA project

cmake_minimum_required(VERSION 3.12)
project(LRA)

## recursive var

# recursive lib files
file(GLOB_RECURSE SRC_LIB ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/*.cpp) 

# recursive driver setting
file(GLOB_RECURSE SRC_DRV2605L_SETTING ${CMAKE_CURRENT_SOURCE_DIR}/src/drv2605L/setting/*.cpp)

## include path
set(INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/inc)
message("====================================================================================")


## compile and link configure
# an example for Driver Setting

add_executable(LRA_Driver_Setting)
target_sources(LRA_Driver_Setting PRIVATE ${SRC_DRV2605L_SETTING})
target_include_directories(LRA_Driver_Setting PRIVATE )
# target_compile_definitions(LRA_Driver_Init std_cxx_17)
