cmake_minimum_required(VERSION 3.16)
project(vnet)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/bin")

# generate debug symbols
add_compile_options(
    "-g"
)

# set CXX flags
set(FLEXNET_CXX_FLAGS_LIST
    ${CMAKE_CXX_FLAGS}
    "-fno-rtti"
)
list(JOIN FLEXNET_CXX_FLAGS_LIST " " CMAKE_CXX_FLAGS)
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

include_directories(
    ${CMAKE_SOURCE_DIR}/3rdparty
)

add_subdirectory(src)
