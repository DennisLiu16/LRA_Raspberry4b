# init main CMakeLists.txt
add_executable(ADXL355_Read_Test ADXL355_Read_Test.cpp)

target_include_directories(ADXL355_Read_Test PRIVATE ${CMAKE_INCLUDE_PATH})

# i2c is from usr/include/i2c lib ; I2C is located in this project repo
target_link_libraries(ADXL355_Read_Test PRIVATE ErrorCode PI ADXL355)
target_compile_definitions(ADXL355_Read_Test PRIVATE std_cxx_17)