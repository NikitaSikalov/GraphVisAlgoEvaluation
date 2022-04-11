project(AestheticMetricsEvaluation)
cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenCV REQUIRED)
#message(STATUS "Use OpenCV lib with version: ${OpenCV_VERSION}")

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(plog/include)
include_directories(lib)

add_subdirectory(lib/OpticalGraphRecognition)
add_subdirectory(lib/optical_graph_recognition)
add_executable(main main.cpp)
target_link_libraries(main ogr)

#add_executable(dev dev.cpp)
#target_link_libraries(dev ${OpenCV_LIBS})
