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

project(SMTStabilizer)

set(BUILD_SHARED_LIBS OFF)

set(SMTSTABILIZER_USE_BUNDLED_STATIC_LIBS_DEFAULT ON)
if (WIN32)
    set(SMTSTABILIZER_USE_BUNDLED_STATIC_LIBS_DEFAULT OFF)
endif()
option(SMTSTABILIZER_USE_BUNDLED_STATIC_LIBS
    "Link against bundled static GMP/MPFR archives under submodules/lib"
    ${SMTSTABILIZER_USE_BUNDLED_STATIC_LIBS_DEFAULT})

# Bitwuzla enablement is automatic when staged headers/libs exist; no CMake
# option is provided to force-enable it.

set(SMTSTABILIZER_FORCE_FULLY_STATIC_DEFAULT OFF)
if (UNIX AND NOT APPLE)
    set(SMTSTABILIZER_FORCE_FULLY_STATIC_DEFAULT ON)
endif()
option(SMTSTABILIZER_FORCE_FULLY_STATIC
    "Request fully static executable linking (Linux only)"
    ${SMTSTABILIZER_FORCE_FULLY_STATIC_DEFAULT})

if (SMTSTABILIZER_FORCE_FULLY_STATIC)
    if (UNIX AND NOT APPLE)
        set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
        set(CMAKE_EXE_LINK_DYNAMIC_C_FLAGS "")
        set(CMAKE_EXE_LINK_DYNAMIC_CXX_FLAGS "")
        set(CMAKE_EXE_LINK_STATIC_C_FLAGS "-static")
        set(CMAKE_EXE_LINK_STATIC_CXX_FLAGS "-static")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
    else()
        message(WARNING "SMTSTABILIZER_FORCE_FULLY_STATIC is only supported on Linux. Ignoring this option on the current platform.")
    endif()
endif()

set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -flto=auto -fPIC")

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(SPEED_FLAGS "-O3 -march=native -mtune=native -funroll-loops -fstrict-aliasing -fomit-frame-pointer -fPIC")
elseif(MSVC)
    set(SPEED_FLAGS "/O2")
else()
    set(SPEED_FLAGS "-O3 -fPIC")
endif()

string(REPLACE ";" " " SPEED_FLAGS "${SPEED_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${SPEED_FLAGS}")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${SPEED_FLAGS}")

option(BUILD_DOCS_ONLY "Only build Doxygen documentation, skip library checks and compilation" OFF)

if(NOT BUILD_DOCS_ONLY)
    enable_testing()
endif()

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

if (NOT BUILD_DOCS_ONLY)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)

    if (SMTSTABILIZER_USE_BUNDLED_STATIC_LIBS)
        include_directories(${CMAKE_CURRENT_SOURCE_DIR}/submodules/include)
        set(ALL_LIBRARIES
            ${CMAKE_SOURCE_DIR}/submodules/lib/libmpfr.a
            ${CMAKE_SOURCE_DIR}/submodules/lib/libgmp.a
            ${CMAKE_SOURCE_DIR}/submodules/lib/libgmpxx.a)
        foreach(LIB_FILE IN LISTS ALL_LIBRARIES)
            if (NOT EXISTS ${LIB_FILE})
                message(FATAL_ERROR
                    "Bundled library not found: ${LIB_FILE}. "
                    "Either initialize bundled dependencies or configure with "
                    "-DSMTSTABILIZER_USE_BUNDLED_STATIC_LIBS=OFF to use system libraries.")
            endif()
        endforeach()
        execute_process(
            COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=libgomp.a
            OUTPUT_VARIABLE GOMP_LIBRARY
            OUTPUT_STRIP_TRAILING_WHITESPACE)
        if (NOT EXISTS ${GOMP_LIBRARY})
            message(FATAL_ERROR "Unable to locate libgomp.a via the active C++ compiler: ${GOMP_LIBRARY}")
        endif()
        set(GOMP_LIB ${GOMP_LIBRARY} CACHE INTERNAL "libgomp library path")
        find_library(DL_LIBRARY dl REQUIRED)
        if (NOT DL_LIBRARY)
            message(FATAL_ERROR "not find libdl")
        else()
            message(STATUS "find libdl: ${DL_LIBRARY}")
        endif()
        set(DL_LIB ${DL_LIBRARY} CACHE INTERNAL "libdl library path")

        # Check for bundled Bitwuzla header and required static libraries.
        set(BITWUZLA_HEADER_BUNDLED ${CMAKE_SOURCE_DIR}/submodules/include/bitwuzla/)
        set(BITWUZLA_LIB_FILES
            ${CMAKE_SOURCE_DIR}/submodules/lib/libbitwuzla.a
            ${CMAKE_SOURCE_DIR}/submodules/lib/libbitwuzlabv.a
            ${CMAKE_SOURCE_DIR}/submodules/lib/libbitwuzlals.a
            ${CMAKE_SOURCE_DIR}/submodules/lib/libbitwuzlabb.a)

        set(BITWUZLA_MISSING)
        if (NOT EXISTS ${BITWUZLA_HEADER_BUNDLED})
            list(APPEND BITWUZLA_MISSING ${BITWUZLA_HEADER_BUNDLED})
        endif()
        foreach(LF IN LISTS BITWUZLA_LIB_FILES)
            if (NOT EXISTS ${LF})
                list(APPEND BITWUZLA_MISSING ${LF})
            endif()
        endforeach()

        if (BITWUZLA_MISSING)
            message(STATUS "Bundled Bitwuzla: missing files: ${BITWUZLA_MISSING}")
            set(BITWUZLA_FOUND OFF)
        else()
            # include_directories(${CMAKE_CURRENT_SOURCE_DIR}/submodules/include)
            # list(APPEND ALL_LIBRARIES ${BITWUZLA_LIB_FILES})
            set(BITWUZLA_FOUND ON)
            message(STATUS "Bundled Bitwuzla: all required files present")
        endif()

        # Check for bundled XGBoost header and required static libraries.
        set(XGBOOST_HEADER_BUNDLED ${CMAKE_SOURCE_DIR}/submodules/include/xgboost/c_api.h)
        set(XGBOOST_LIB_FILES
            ${CMAKE_SOURCE_DIR}/submodules/lib/xgboost/libxgboost.a
            ${CMAKE_SOURCE_DIR}/submodules/lib/xgboost/libdmlc.a)

        set(XGBOOST_MISSING)
        if (NOT EXISTS ${XGBOOST_HEADER_BUNDLED})
            list(APPEND XGBOOST_MISSING ${XGBOOST_HEADER_BUNDLED})
        endif()
        foreach(LF IN LISTS XGBOOST_LIB_FILES)
            if (NOT EXISTS ${LF})
                list(APPEND XGBOOST_MISSING ${LF})
            endif()
        endforeach()

        if (XGBOOST_MISSING)
            message(STATUS "Bundled XGBoost: missing files: ${XGBOOST_MISSING}")
            set(XGBOOST_FOUND OFF)
        else()
            set(XGBOOST_FOUND ON)
            list(APPEND ALL_LIBRARIES ${XGBOOST_LIB_FILES})
            message(STATUS "Bundled XGBoost: all required files present")
        endif()
    else()
        find_path(GMP_INCLUDE_DIR gmp.h)
        find_path(MPFR_INCLUDE_DIR mpfr.h)
        find_library(GMP_LIBRARY NAMES gmp libgmp)
        find_library(GMPXX_LIBRARY NAMES gmpxx libgmpxx)
        find_library(MPFR_LIBRARY NAMES mpfr libmpfr)

        if (NOT GMP_INCLUDE_DIR OR NOT MPFR_INCLUDE_DIR OR NOT GMP_LIBRARY OR NOT GMPXX_LIBRARY OR NOT MPFR_LIBRARY)
            message(FATAL_ERROR
                "Unable to find GMP/MPFR development files. "
                "Install platform packages (Linux: libgmp-dev libmpfr-dev, macOS: brew install gmp mpfr), "
                "or on Windows use vcpkg and pass its toolchain file to CMake.")
        endif()

        include_directories(${GMP_INCLUDE_DIR} ${MPFR_INCLUDE_DIR})
        set(ALL_LIBRARIES ${MPFR_LIBRARY} ${GMPXX_LIBRARY} ${GMP_LIBRARY})
    endif()

    # Bitwuzla detection is only supported via bundled staged headers/libs
    # placed by `setup.py` (i.e. submodules/include and submodules/lib).
    # Do not attempt to enable Bitwuzla via a CMake flag alone.
    if (BITWUZLA_FOUND)
        add_compile_definitions(SMTSTABILIZER_HAVE_BITWUZLA)
        message(STATUS "Bitwuzla support: enabled (bundled)")
    else()
        message(STATUS "Bitwuzla support: disabled")
    endif()

    # XGBoost detection is only supported via bundled staged headers/libs
    # placed by `setup.py` (submodules/include/xgboost and submodules/lib/xgboost).
    if (XGBOOST_FOUND)
        add_compile_definitions(SMTSTABILIZER_HAVE_XGBOOST)
        message(STATUS "XGBoost support: enabled (bundled)")
    else()
        message(STATUS "XGBoost support: disabled")
    endif()


    # add_subdirectory(src/kernel)
    add_subdirectory(src/solver)
    add_subdirectory(src/main)
    add_subdirectory(src/api)
    add_subdirectory(src/util)
    # add_subdirectory(src/rewrite)
    add_subdirectory(src/parser)
    add_subdirectory(src/kernel)
    add_subdirectory(src/node)
    add_subdirectory(src/option)
    add_subdirectory(test)
endif()