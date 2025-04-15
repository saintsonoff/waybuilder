add_library(output_manager STATIC output_manager.cpp)

include(FetchContent)

# nlohmann/json connecting
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

# boost/log connecting
set(BOOST_INCLUDE_LIBRARIES log)
set(BOOST_ENABLE_CMAKE ON)
FetchContent_Declare(
  Boost
  URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.tar.xz
  URL_MD5 893b5203b862eb9bbd08553e24ff146a
)
FetchContent_MakeAvailable(Boost)

target_link_libraries(output_manager PUBLIC Boost::log)
target_link_libraries(output_manager PUBLIC nlohmann_json::nlohmann_json)

target_link_libraries(output_manager PUBLIC ya_rasp_json_ptr)
target_link_libraries(output_manager PRIVATE ya_rasp_cli)

target_include_directories(output_manager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})