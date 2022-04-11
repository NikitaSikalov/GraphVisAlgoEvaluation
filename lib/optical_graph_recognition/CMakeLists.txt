find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(.)

add_library(ogr2 optical_graph_recognition.cpp)

target_link_libraries(ogr2 ${OpenCV_LIBS})
