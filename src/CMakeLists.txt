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

if (ENABLE_GPERF)
    add_compile_definitions(
        ENABLE_GPERF
    )
    link_libraries(profiler)
endif()

#####################################################
# create vn-agent-select-rdwr target
project(vn-agent-select-rdwr)
add_executable(${PROJECT_NAME}
    ${SRC_vnet}
)

#####################################################
# create vn-agent-epoll-rdwr target
project(vn-agent-epoll-rdwr)
add_executable(${PROJECT_NAME}
    ${SRC_vnet}
)
target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        -DSOCKET_EPOLL
)

#####################################################
# create vn-agent-select-splice target
project(vn-agent-select-splice)
add_executable(${PROJECT_NAME}
    ${SRC_vnet}
)
target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        -DSPLICE_TRANSFER
)

#####################################################
# create vn-agent-epoll-splice target
project(vn-agent-epoll-splice)
add_executable(${PROJECT_NAME}
    ${SRC_vnet}
)
target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        -DSOCKET_EPOLL
        -DSPLICE_TRANSFER
)
