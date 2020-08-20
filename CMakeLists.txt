cmake_minimum_required(VERSION 3.16)
project(ScreenFramer VERSION 1.0.0)

# dependencies
find_package(OpenCV 4 REQUIRED)
find_package(cxxopts 2 REQUIRED)
find_package(nlohmann_json 3.8 REQUIRED)

# compilation options
set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall)
add_compile_options(-Wextra)
add_compile_options(-pedantic)

# configiration
set(SF_BINARY_NAME "screenframer")
set(SF_BINARY_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin")
set(SF_DATA_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/${SF_BINARY_NAME}")
set(SF_TEMPLATES_INSTALL_DIR "${SF_DATA_INSTALL_DIR}/devices")

# definitions
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_compile_definitions(DEBUG)
    add_compile_definitions(RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/Resources")
else()
    add_compile_definitions(RELEASE)
    add_compile_definitions(RESOURCES_PATH="${SF_TEMPLATES_INSTALL_DIR}")
    message(STATUS "Resource dir: ${SF_TEMPLATES_INSTALL_DIR}")
endif()
add_compile_definitions(VERSION_NUMBER="${PROJECT_VERSION}")

# sources and link libraries
set(ScreenFramer_SOURCES
        Sources/Overlayer.cpp
        Sources/OverlayTask.cpp
        Sources/OutputConfig.cpp
        Sources/main.cpp)
add_executable(ScreenFramer ${ScreenFramer_SOURCES})
target_link_libraries(ScreenFramer ${OpenCV_LIBS})
target_link_libraries(ScreenFramer cxxopts::cxxopts)
target_link_libraries(ScreenFramer nlohmann_json::nlohmann_json)
target_include_directories(ScreenFramer PRIVATE ./Dependencies/cpptqdm)
set_target_properties(ScreenFramer PROPERTIES OUTPUT_NAME ${SF_BINARY_NAME})

# installation
install(TARGETS ScreenFramer DESTINATION ${SF_BINARY_INSTALL_DIR})
install(DIRECTORY Resources/ CONFIGURATIONS Release DESTINATION ${SF_TEMPLATES_INSTALL_DIR} PATTERN "Resources/*")