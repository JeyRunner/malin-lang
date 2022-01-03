cmake_minimum_required(VERSION 3.14)

# get version from git
include(cmake/versionFromGit/VersionFromGit.cmake)
version_from_git(
        #LOG       ON
        #TIMESTAMP "%Y%m%d%H%M%S"
)
message(STATUS "malinc build version: ${VERSION}  (commit: ${GIT_HASH})")
message("")


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
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# add_compile_options(-fmodules)

# #######################
# dependencies
#

# lyra
set(DEPS_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/deps)
ExternalProject_Add(
        lyra
        GIT_REPOSITORY https://github.com/bfgroup/Lyra.git
        GIT_TAG 1.5.1
        PREFIX ${DEPS_INSTALL_LOCATION}/lyra
        INSTALL_COMMAND ""
        #CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${DEPS_INSTALL_LOCATION}
)

# magic_enum
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
#hunter_add_package(Boost)
#find_package(Boost CONFIG REQUIRED)


###################
## llvm
option(DEP_LLVM_LINK_SHARED "When turned on, llvm is linked as shared lib. When turned off, llvm is linkend statically" ON)
if (DEP_LLVM_LINK_SHARED)
    message(STATUS "[LLVM] linking shared")
    set(LLVM_CONF_LINK_FLAG --link-shared)
else()
    message(STATUS "[LLVM] linking static")
    set(LLVM_CONF_LINK_FLAG --link-static)
endif()


find_package(LLVM 10 CONFIG REQUIRED)
#llvm_map_components_to_libnames(llvm_libs support core target codegen all-targets) # core CodeGen Target native all)

# get llvm libs
set(LLVM_COMPONENTS support core target codegen all-targets)
# llvm static libs
execute_process(COMMAND llvm-config-10  ${LLVM_CONF_LINK_FLAG} --libs all
        OUTPUT_VARIABLE LLVM_LIBS_OF_COMPONENTS)
# required for statically linking to llvm
execute_process(COMMAND llvm-config-10  ${LLVM_CONF_LINK_FLAG} --system-libs
        OUTPUT_VARIABLE LLVM_DEP_LIBS)

# clean output
string(REGEX REPLACE "\n$" "" LLVM_LIBS_OF_COMPONENTS "${LLVM_LIBS_OF_COMPONENTS}")
string(REGEX REPLACE "\n$" "" LLVM_DEP_LIBS "${LLVM_DEP_LIBS}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
message(STATUS "[LLVM] found version ${LLVM_PACKAGE_VERSION}, llvm cmake config is in folder ${LLVM_DIR}")
message(STATUS "[LLVM] include: ${LLVM_INCLUDE_DIRS}")
message(STATUS "[LLVM] defs: ${LLVM_DEFINITIONS}")
message(STATUS "[LLVM] libs: ${LLVM_LIBS_OF_COMPONENTS}")
message(STATUS "[LLVM] system-libs: ${LLVM_DEP_LIBS}")
message(STATUS "[LLVM] lib dir: ${LLVM_LIBRARY_DIR}")



set(DEPENDENCIES lyra magic_enum malinCGlue)
#hunter_add_package(spdlog)
#find_package(spdlog CONFIG REQUIRED)
#target_link_libraries( spdlog::spdlog)



# files
file(GLOB SRC_FILES src/*.cpp src/*/*.cpp)
configure_file(src/util/version.h.in util/version.h)
message(STATUS "[BUILD] src files: ${SRC_FILES} ")

add_executable(malinc ${SRC_FILES})
add_dependencies(malinc ${DEPENDENCIES})

include_directories(${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(malinc stdc++fs termcolor::termcolor ${LLVM_LIBS_OF_COMPONENTS} ${LLVM_DEP_LIBS}) #Boost::boost) # ${llvm_libs} # LLVM
target_link_directories(malinc PUBLIC ${LLVM_LIBRARY_DIR})


# test executable
#add_executable(malinc-tests test/cpp/AstNodeChildrenIteratorTest.cpp)
#target_link_libraries(malinc-tests stdc++fs) # ${llvm_libs}





# install
install(TARGETS malinc malinCGlue
        COMPONENT malinc

        RUNTIME DESTINATION bin ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION lib ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION lib ${CMAKE_INSTALL_LIBDIR}
        )

SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Jeyrunner")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "clang") # libllvm10

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Malin language compiler")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

SET(CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}")

INCLUDE(CPack)
