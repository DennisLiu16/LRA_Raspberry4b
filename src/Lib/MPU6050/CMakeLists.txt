file(GLOB_RECURSE SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
# add library
add_library(MPU6050 SHARED ${SRC})
target_include_directories(MPU6050 PUBLIC ${CMAKE_INCLUDE_PATH})
