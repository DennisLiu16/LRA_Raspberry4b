add_executable(TCPIP_Client LRA_Client.c)
target_include_directories(TCPIP_Client PRIVATE ${CMAKE_INCLUDE_PATH})
target_compile_definitions(TCPIP_Client PRIVATE std_c_17)

add_executable(TCPIP_Server LRA_Server.c)
target_include_directories(TCPIP_Server PRIVATE ${CMAKE_INCLUDE_PATH})
target_compile_definitions(TCPIP_Server PRIVATE std_c_17)
target_link_libraries( TCPIP_Server 
    pthread
)

