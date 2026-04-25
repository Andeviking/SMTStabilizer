# Copyright (c) 2026 XiangZhang
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

cmake_minimum_required(VERSION 3.16)

set(BUILD_SHARED_LIBS OFF)
set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
set(CMAKE_EXE_LINK_DYNAMIC_C_FLAGS "")
set(CMAKE_EXE_LINK_DYNAMIC_CXX_FLAGS "")
set(CMAKE_EXE_LINK_STATIC_C_FLAGS "-static")
set(CMAKE_EXE_LINK_STATIC_CXX_FLAGS "-static")

if (UNIX AND NOT APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
endif()

set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -flto=auto -fPIC")

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(SPEED_FLAGS "-O3 -march=native -mtune=native -funroll-loops -fstrict-aliasing -fomit-frame-pointer -fPIC")
else()
    set(SPEED_FLAGS "-O3 -fPIC")
endif()

string(REPLACE ";" " " SPEED_FLAGS "${SPEED_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${SPEED_FLAGS}")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${SPEED_FLAGS}")

set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(BUILD_SHARED_LIBS OFF)        

project(SMTStabilizer)

enable_testing()

find_package(Doxygen QUIET)
if (DOXYGEN_FOUND)
    set(DOXYGEN_INPUT_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/api
        ${CMAKE_CURRENT_SOURCE_DIR}/src/kernel
        ${CMAKE_CURRENT_SOURCE_DIR}/src/parser
        ${CMAKE_CURRENT_SOURCE_DIR}/src/util
        ${CMAKE_CURRENT_SOURCE_DIR}/docs/pages
        ${CMAKE_CURRENT_SOURCE_DIR}/README.md)
    string(REPLACE ";" " " DOXYGEN_INPUT "${DOXYGEN_INPUT_DIRS}")
    set(DOXYGEN_MAINPAGE ${CMAKE_CURRENT_SOURCE_DIR}/docs/pages/mainpage.md)
    set(DOXYGEN_TAGFILE ${CMAKE_CURRENT_BINARY_DIR}/SMTStabilizer.tag)
    set(DOXYGEN_WARN_LOGFILE ${CMAKE_CURRENT_BINARY_DIR}/doxygen_warnings.log)
    set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/docs)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)
else()
    add_custom_target(docs
        COMMAND ${CMAKE_COMMAND} -E echo "Doxygen was not found; install doxygen to build the docs target."
        COMMAND ${CMAKE_COMMAND} -E false
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/submodules/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# file(GLOB_RECURSE ALL_LIBRARIES "${CMAKE_SOURCE_DIR}/submodules/lib/*.a")
set(ALL_LIBRARIES
    ${CMAKE_SOURCE_DIR}/submodules/lib/libmpfr.a
    ${CMAKE_SOURCE_DIR}/submodules/lib/libgmp.a
    ${CMAKE_SOURCE_DIR}/submodules/lib/libgmpxx.a)

# add_subdirectory(src/kernel)
add_subdirectory(src/main)
add_subdirectory(src/api)
add_subdirectory(src/util)
# add_subdirectory(src/rewrite)
add_subdirectory(src/parser)
add_subdirectory(src/kernel)
add_subdirectory(src/node)
add_subdirectory(test)