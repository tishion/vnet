include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

file(GLOB_RECURSE SRC_vnet
    common/*
    base/*
    app/*
)

source_group(
    TREE ${CMAKE_CURRENT_SOURCE_DIR}
    FILES ${SRC_vnet}
)

link_libraries(pthread)

#####################################################
# create vn-agent-select target
project(vn-agent-select)
add_executable(${PROJECT_NAME}
    ${SRC_vnet}
)

#####################################################
# create vn-agent-epoll target
project(vn-agent-epoll)
add_executable(${PROJECT_NAME}
    ${SRC_vnet}
)
target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        -DSOCKET_EPOLL
)
