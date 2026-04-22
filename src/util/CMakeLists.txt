set(LIB_NAME util)
file(GLOB_RECURSE SRC
    "*.cpp"
    "*.h"
    "*.hpp"
)

add_library(${LIB_NAME} ${SRC})