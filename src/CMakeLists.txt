include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

file(GLOB_RECURSE SRC_vnet
    common/*
    base/*
)
source_group(
    TREE ${CMAKE_CURRENT_SOURCE_DIR}
    FILES ${SRC_vnet}
)

project(vnet-agent)

file(GLOB_RECURSE SRC_vnet_agent
    app/vnet-agent/*
)
source_group(app
    FILES ${SRC_vnet_agent}
)

add_executable(${PROJECT_NAME}
    ${SRC_vnet_agent}
    ${SRC_vnet}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        pthread
)

