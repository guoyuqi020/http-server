cmake_minimum_required (VERSION 3.0)
file(GLOB HANDLER_SRCS "*.c")
add_library(handler_lib STATIC ${HANDLER_SRCS})
