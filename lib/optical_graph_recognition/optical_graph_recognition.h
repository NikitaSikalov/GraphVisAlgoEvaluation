#pragma once

#include <opencv2/opencv.hpp>

#include <ogr_components/matrix.h>
#include <iterators/consecutive_iterator.h>

#include <vertex/detectors.h>

namespace ogr {
    class OpticalGraphRecognition {
    public:
        explicit OpticalGraphRecognition(const cv::Mat& source_graph);

        void DetectVertexes(std::function<bool(point::PointPtr)> is_vertex);
        void DetectEdges();
    private:
        matrix::GraphRecognitionMatrix grm_;
        std::vector<VertexPtr> vertexes_;
        std::vector<EdgePtr> edges_;
    };
}
