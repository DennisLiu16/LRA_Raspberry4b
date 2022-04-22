# init main CMakeLists.txt
add_executable(Single_DRV_TCA_tcpiptest TCPIP_with_DRV.cpp)

target_include_directories(Single_DRV_TCA_tcpiptest PRIVATE ${CMAKE_INCLUDE_PATH})

# i2c is from usr/include/i2c lib ; I2C is located in this project repo
target_link_libraries(Single_DRV_TCA_tcpiptest PUBLIC DRV2605L_TCA ADXL355 ErrorCode PI I2C TCPIP)
target_compile_definitions(Single_DRV_TCA_tcpiptest PRIVATE std_cxx_17)