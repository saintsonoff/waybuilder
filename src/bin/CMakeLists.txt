add_executable(waybuilder main.cpp)

target_link_libraries(waybuilder PRIVATE console_cli_app)
target_link_libraries(waybuilder PRIVATE ya_rasp_cli)