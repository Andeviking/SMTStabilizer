set(LIB_NAME node)
file(GLOB_RECURSE SRC
    "*.cpp"
    "*.h"
)

add_library(${LIB_NAME} ${SRC})