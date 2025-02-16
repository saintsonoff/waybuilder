add_library(command_module INTERFACE)

target_include_directories(command_module INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(command_fabric INTERFACE)

target_include_directories(command_fabric INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)