find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
#include_directories(.)

add_library(ogr
        optical_graph_recognition.cpp
        reporter.cpp
        algo_params/params.cpp
        crawler/step.cpp
        crawler/edge_crawler.cpp
        crawler/edges_detector.cpp
        crawler/step_tree_node.cpp
        utils/debug.cpp
        utils/opencv_utils.cpp
        stats/stats.cpp
        )

target_link_libraries(ogr ${OpenCV_LIBS})
