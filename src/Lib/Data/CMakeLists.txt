file(GLOB_RECURSE SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
# add library
add_library(Data SHARED ${SRC})
list(APPEND CMAKE_PREFIX_PATH "/usr/local/lib/mariadb")
find_library(MariaDB_LIB lmariadbcpp)

target_link_libraries(Data ${MariaDB_LIB})
target_include_directories(Data PUBLIC ${CMAKE_INCLUDE_PATH})
target_compile_features(Data PUBLIC cxx_std_17)
