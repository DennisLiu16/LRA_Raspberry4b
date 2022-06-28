file(GLOB_RECURSE SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
# add library
add_library(ADXL355 SHARED ${SRC})
find_library(wiringPi_LIB wiringPi)

target_link_libraries(ADXL355 ${wiringPi_LIB})
target_include_directories(ADXL355 PRIVATE ${CMAKE_INCLUDE_PATH})
target_compile_features(ADXL355 PUBLIC cxx_std_17)
