set(LIB_NAME parser)
file(GLOB_RECURSE SRC
    "*.cpp"
    "*.h"
)

add_library(${LIB_NAME} ${SRC})