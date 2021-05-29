cmake_minimum_required(VERSION 3.2)

# get version from git
include(cmake/versionFromGit/VersionFromGit.cmake)
version_from_git(
        #LOG       ON
        #TIMESTAMP "%Y%m%d%H%M%S"
)
message(STATUS "build version ${VERSION}")


# package manager
include(ExternalProject)
include("cmake/HunterGate.cmake")
HunterGate(
        URL "https://github.com/cpp-pm/hunter/archive/v0.23.240.tar.gz"
        SHA1 "ca19f3769e6c80cfdd19d8b12ba5102c27b074e0"
)
add_subdirectory(std/c)


# project
project(malinc)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# #######################
# dependencies
#
set(DEPS_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/deps)
ExternalProject_Add(
        lyra
        GIT_REPOSITORY https://github.com/bfgroup/Lyra.git
        GIT_TAG 1.5.1
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
        src/
)
# termcolor
hunter_add_package(termcolor)
find_package(termcolor CONFIG REQUIRED)
# boost
hunter_add_package(Boost)
find_package(Boost CONFIG REQUIRED)


# llvm
find_package(LLVM CONFIG REQUIRED)
llvm_map_components_to_libnames(llvm_libs support core CodeGen Target native)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
message(STATUS "[LLVM] include: ${LLVM_INCLUDE_DIRS}")
message(STATUS "[LLVM] defs: ${LLVM_DEFINITIONS}")
message(STATUS "[LLVM] libs: ${llvm_libs}")

set(DEPENDENCIES lyra magic_enum)
#hunter_add_package(spdlog)
#find_package(spdlog CONFIG REQUIRED)
#target_link_libraries( spdlog::spdlog)



# files
file(GLOB SRC_FILES src/*.cpp src/*/*.cpp)
message("src files: ${SRC_FILES} ")
add_executable(malinc ${SRC_FILES})
add_dependencies(malinc ${DEPENDENCIES})
target_link_libraries(malinc stdc++fs termcolor::termcolor LLVM Boost::boost) # ${llvm_libs}


# test executable
#add_executable(malinc-tests test/cpp/AstNodeChildrenIteratorTest.cpp)
#target_link_libraries(malinc-tests stdc++fs) # ${llvm_libs}





# install
install(TARGETS malinc malinCGlue
        COMPONENT malinc
        #RUNTIME DESTINATION "/usr/bin"
        #LIBRARY DESTINATION "/home/"
        #DESTINATION "/home/"
        )

SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Jeyrunner")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libllvm9, clang-9") #(>= 6.0)

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Malin language compiler")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

SET(CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}")

INCLUDE(CPack)
