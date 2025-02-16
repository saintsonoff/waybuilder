add_library(ya_rasp_cli STATIC ya_rasp_cli.cpp)


include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
                         GIT_TAG dec9422db3af470641f8b0d90e4b451c4daebf64) # Replace with your desired git commit from: https://github.com/libcpr/cpr/releases
FetchContent_MakeAvailable(cpr)


target_link_libraries(ya_rasp_cli PUBLIC cpr::cpr)
target_link_libraries(ya_rasp_cli PUBLIC nlohmann_json::nlohmann_json)

target_include_directories(ya_rasp_cli PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})