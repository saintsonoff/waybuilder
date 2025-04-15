include(FetchContent)

# nlohmann/json connecting
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

# libcpr/cpr connecting
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
GIT_TAG dec9422db3af470641f8b0d90e4b451c4daebf64) # Replace with your desired git commit from: https://github.com/libcpr/cpr/releases
FetchContent_MakeAvailable(cpr)


# boost/log connecting
set(BOOST_INCLUDE_LIBRARIES log)
set(BOOST_ENABLE_CMAKE ON)
FetchContent_Declare(
  Boost
  URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.tar.xz
  URL_MD5 893b5203b862eb9bbd08553e24ff146a
)
FetchContent_MakeAvailable(Boost)

add_library(ya_rasp_json_ptr STATIC ya_rasp_json_ptr.cpp)

target_link_libraries(ya_rasp_json_ptr PUBLIC nlohmann_json::nlohmann_json)
target_include_directories(ya_rasp_json_ptr PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


add_library(ya_rasp_cli STATIC ya_rasp_cli.cpp)

target_link_libraries(ya_rasp_cli PUBLIC cpr::cpr)
target_link_libraries(ya_rasp_cli PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(ya_rasp_cli PUBLIC Boost::log Boost::log_setup)

target_link_libraries(ya_rasp_cli PRIVATE ya_rasp_json_ptr)

target_include_directories(ya_rasp_cli PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})