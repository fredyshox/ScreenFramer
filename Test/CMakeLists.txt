cmake_minimum_required(VERSION 3.16)

macro(add_unit_test target args)
    add_test(NAME "${target}"
            COMMAND "${target}" ${args}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endmacro()

add_executable(SFBenchmark benchmark.cpp ../Sources/Overlayer.cpp ../Sources/OverlayTask.cpp)
target_link_libraries(SFBenchmark ${OpenCV_LIBS})
target_include_directories(SFBenchmark PRIVATE ../Sources)
add_unit_test(SFBenchmark "")
