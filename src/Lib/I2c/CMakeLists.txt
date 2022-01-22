file(GLOB_RECURSE SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
# add library
add_library(I2C SHARED ${SRC})
target_include_directories(I2C PUBLIC ${CMAKE_INCLUDE_PATH})
target_compile_features(I2C PUBLIC c_std_17)
