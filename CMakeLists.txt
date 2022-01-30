project(AestheticMetricsEvaluation)
cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenCV REQUIRED)
message(STATUS "Use OpenCV lib with version: ${OpenCV_VERSION}")

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(plog/include)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/images/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/images/)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/output/)

add_executable(main main.cpp)
target_link_libraries(main ${OpenCV_LIBS})
