cmake_minimum_required(VERSION 3.16)
project(AppVideoFramer VERSION 1.0.0)

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
set(AVF_BINARY_NAME "avframer")
set(AVF_BINARY_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin")
set(AVF_DATA_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/${AVF_BINARY_NAME}")
set(AVF_TEMPLATES_INSTALL_DIR "${AVF_DATA_INSTALL_DIR}/devices")

# definitions
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_compile_definitions(DEBUG)
    add_compile_definitions(RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/Resources")
else()
    add_compile_definitions(RELEASE)
    add_compile_definitions(RESOURCES_PATH="${AVF_TEMPLATES_INSTALL_DIR}")
    message(STATUS "Resource dir: ${AVF_TEMPLATES_INSTALL_DIR}")
endif()
add_compile_definitions(VERSION_NUMBER="${PROJECT_VERSION}")

# sources and link libraries
set(AppVideoFramer_SOURCES
        Sources/Overlayer.cpp
        Sources/OverlayTask.cpp
        Sources/main.cpp)
add_executable(AppVideoFramer ${AppVideoFramer_SOURCES})
target_link_libraries(AppVideoFramer ${OpenCV_LIBS})
target_link_libraries(AppVideoFramer cxxopts::cxxopts)
target_link_libraries(AppVideoFramer nlohmann_json::nlohmann_json)
target_include_directories(AppVideoFramer PRIVATE ./Dependencies/cpptqdm)
set_target_properties(AppVideoFramer PROPERTIES OUTPUT_NAME ${AVF_BINARY_NAME})

# installation
install(TARGETS AppVideoFramer DESTINATION ${AVF_BINARY_INSTALL_DIR})
install(DIRECTORY Resources/ CONFIGURATIONS Release DESTINATION ${AVF_TEMPLATES_INSTALL_DIR} PATTERN "Resources/*")