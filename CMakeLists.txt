project(AestheticMetricsEvaluation)
cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenCV REQUIRED)
message(STATUS "Use OpenCV lib with version: ${OpenCV_VERSION}")

# Deps
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(plog/include)
include_directories(cli_parser/include)
include_directories(thread_pool)
include_directories(tabulate/include)

# Source libraries
include_directories(lib)
include_directories(lib/optical_graph_recognition)

add_subdirectory(lib/optical_graph_recognition)
add_executable(main main.cpp)
target_link_libraries(main ogr)

#add_executable(dev dev.cpp)
#target_link_libraries(dev ${OpenCV_LIBS})
