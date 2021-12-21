file(GLOB_RECURSE SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
# add library
add_library(ADXL355 SHARED ${SRC})
target_include_directories(ADXL355 PUBLIC ${CMAKE_INCLUDE_PATH})
