cmake_minimum_required (VERSION 3.0)
project (HTTP-SERVER)
include_directories(include)

add_subdirectory(lib)

SET(EXECUTABLE_OUTPUT_PATH ..) 
add_executable(server server.c)
target_link_libraries(server handler_lib event ssl crypto)