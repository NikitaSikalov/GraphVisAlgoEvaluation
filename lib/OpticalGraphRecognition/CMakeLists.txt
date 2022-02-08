find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_library(ogr extended_mat.cpp)

target_link_libraries(ogr ${OpenCV_LIBS})
