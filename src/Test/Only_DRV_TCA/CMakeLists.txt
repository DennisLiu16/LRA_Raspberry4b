# init main CMakeLists.txt
add_executable(Only_DRV_TCA Only_DRV_TCA.cpp)

target_include_directories(Only_DRV_TCA PRIVATE ${CMAKE_INCLUDE_PATH})

# i2c is from usr/include/i2c lib ; I2C is located in this project repo
target_link_libraries(Only_DRV_TCA PUBLIC DRV2605L_TCA ErrorCode PI I2C)
target_compile_definitions(Only_DRV_TCA PRIVATE std_cxx_17)