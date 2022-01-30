project(AestheticMetricsEvaluation)
cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenCV REQUIRED)
message(STATUS "Found and loaded OpenCV lib with version: ${OpenCV_VERSION}")

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(main main.cpp)
target_link_libraries(main ${OpenCV_LIBS})
