cmake_minimum_required(VERSION 3.2)

# package manager

include(ExternalProject)
include("cmake/HunterGate.cmake")
HunterGate(
        URL "https://github.com/cpp-pm/hunter/archive/v0.23.240.tar.gz"
        SHA1 "ca19f3769e6c80cfdd19d8b12ba5102c27b074e0"
)


# project
project(own_compiler)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# #######################
# dependencies
#
set(DEPS_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/deps)
ExternalProject_Add(
        lyra
        GIT_REPOSITORY https://github.com/bfgroup/Lyra.git
        PREFIX ${DEPS_INSTALL_LOCATION}/lyra
        INSTALL_COMMAND ""
        #CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${DEPS_INSTALL_LOCATION}
)
ExternalProject_Add(
        magic_enum
        GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
        PREFIX ${DEPS_INSTALL_LOCATION}/magic_enum
        INSTALL_COMMAND ""
        #CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${DEPS_INSTALL_LOCATION}
)
include_directories(
        ${DEPS_INSTALL_LOCATION}/lyra/src/lyra/include/
        ${DEPS_INSTALL_LOCATION}/magic_enum/src/magic_enum/include/
)
set(DEPENDENCIES lyra magic_enum)
#hunter_add_package(spdlog)
#find_package(spdlog CONFIG REQUIRED)
#target_link_libraries( spdlog::spdlog)



# files
file(GLOB SRC_FILES src/*.cpp)
add_executable(own_compiler ${SRC_FILES})
add_dependencies(own_compiler ${DEPENDENCIES})
target_link_libraries(own_compiler stdc++fs)