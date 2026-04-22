file(GLOB_RECURSE SRC
    "*.cpp"
    "*.h"
    "*.hpp"
)

# Set the output directory for the executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add the executable
add_executable(${PROJECT_NAME} ${SRC})

target_link_libraries(${PROJECT_NAME} kernel node parser util ${ALL_LIBRARIES})