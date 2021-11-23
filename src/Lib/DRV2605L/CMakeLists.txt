file(GLOB_RECURSE SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
# add library
add_library(DRV2605L SHARED ${SRC})
find_library(wiringPi_LIB wiringPi)
target_link_libraries(DRV2605L ${wiringPi_LIB})
target_include_directories(DRV2605L PUBLIC ${CMAKE_INCLUDE_PATH})
