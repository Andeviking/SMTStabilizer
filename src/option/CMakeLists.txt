# Set the library name
set(OPTION_LIB_NAME option)

# Find all source files in the option directory
file(GLOB_RECURSE OPTION_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE OPTION_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

# Add the option library
add_library(${OPTION_LIB_NAME} ${OPTION_SOURCES} ${OPTION_HEADERS})

