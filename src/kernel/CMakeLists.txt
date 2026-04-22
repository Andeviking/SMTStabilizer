set(LIB_NAME kernel)
file(GLOB_RECURSE SRC
    "*.cpp"
    "*.h"
)

add_library(${LIB_NAME} ${SRC})