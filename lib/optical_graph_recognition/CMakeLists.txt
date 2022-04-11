find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
#include_directories(.)

add_library(ogr optical_graph_recognition.cpp)

target_link_libraries(ogr ${OpenCV_LIBS})
