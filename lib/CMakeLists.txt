cmake_minimum_required (VERSION 3.0)
include_directories(include)
file(GLOB HANDLER_SRCS "*.c")
add_library(handler_lib STATIC ${HANDLER_SRCS})
