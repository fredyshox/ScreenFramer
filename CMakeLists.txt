cmake_minimum_required(VERSION 3.16)
project(AppleVideoOverlay VERSION 1.0.0)

# dependencies
find_package(OpenCV 4 REQUIRED)
find_package(cxxopts 2 REQUIRED)
find_package(nlohmann_json 3.8 REQUIRED)

# compilation options
set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall)
add_compile_options(-Wextra)
add_compile_options(-pedantic)

# definitions
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_compile_definitions(DEBUG)
    add_compile_definitions(RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/Resources")
else()
    add_compile_definitions(RELEASE)
    message(FATAL_ERROR "Relase build not implemented")
endif()
add_compile_definitions(VERSION_NUMBER="${PROJECT_VERSION}")

# sources and link libraries
set(AppleVideoOverlay_SOURCES
        Sources/Overlayer.cpp
        Sources/OverlayTask.cpp
        Sources/main.cpp)
add_executable(AppleVideoOverlay ${AppleVideoOverlay_SOURCES})
target_link_libraries(AppleVideoOverlay ${OpenCV_LIBS})
target_link_libraries(AppleVideoOverlay cxxopts::cxxopts)
target_link_libraries(AppleVideoOverlay nlohmann_json::nlohmann_json)
target_include_directories(AppleVideoOverlay PRIVATE ./Dependencies/cpptqdm)