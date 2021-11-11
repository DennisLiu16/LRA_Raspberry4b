file(GLOB_RECURSE SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
# add library
add_library(PI SHARED ${SRC})
target_include_directories(PI PUBLIC ${CMAKE_INCLUDE_PATH})
