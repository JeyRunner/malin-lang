cmake_minimum_required(VERSION 3.2)
project(malinCGlue)


file(GLOB SRC_FILES *.cpp)
add_library(malinCGlue ${SRC_FILES})

set_target_properties(malinCGlue PROPERTIES DEBUG_POSTFIX "")