#pragma once

#include <opencv2/opencv.hpp>

#include <ogr_components/matrix.h>
#include <iterators/consecutive_iterator.h>

namespace ogr {
    class OpticalGraphRecognition {
    public:
        explicit OpticalGraphRecognition(const cv::Mat& source_graph);

        void DetectVertexes();
        void DetectEdges();
    private:
        matrix::GraphRecognitionMatrix matrix_;
        std::vector<VertexPtr> vertexes_;
        std::vector<EdgePtr> edges_;
    };
}
