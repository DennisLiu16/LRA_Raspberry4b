# init main CMakeLists.txt
add_executable(Single_DRV_TCA_readtest Single_DRV_TCA_readtest.cpp)

target_include_directories(Single_DRV_TCA_readtest PRIVATE ${CMAKE_INCLUDE_PATH})

# i2c is from usr/include/i2c lib ; I2C is located in this project repo
target_link_libraries(Single_DRV_TCA_readtest PUBLIC DRV2605L_TCA ADXL355 ErrorCode PI I2C)
target_compile_definitions(Single_DRV_TCA_readtest PRIVATE std_cxx_17)