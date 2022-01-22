file(GLOB_RECURSE SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
# add library
add_library(ErrorCode SHARED ${SRC})
target_include_directories(ErrorCode PUBLIC ${CMAKE_INCLUDE_PATH})
target_compile_features(ErrorCode PUBLIC cxx_std_17)