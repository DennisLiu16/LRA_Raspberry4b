# init main CMakeLists.txt
add_executable(LRA_Init LRA_DRV2605L_Init.cpp)

target_include_directories(LRA_Init PRIVATE ${CMAKE_INCLUDE_PATH})
target_link_libraries(LRA_Init PRIVATE DRV2605L ErrorCode PI)
target_compile_definitions(LRA_Init PRIVATE std_cxx_17)