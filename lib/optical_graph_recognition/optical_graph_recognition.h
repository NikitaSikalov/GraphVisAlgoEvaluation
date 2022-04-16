#pragma once

#include <ogr_components/matrix.h>
#include <iterators/consecutive_iterator.h>
#include <vertex/detectors.h>
#include <utils/debug.h>

#include <opencv2/opencv.hpp>

#include <unordered_map>
#include <optional>


namespace ogr {
    class OpticalGraphRecognition {
    public:
        explicit OpticalGraphRecognition(const cv::Mat& source_graph);

        void DetectVertexes(std::function<bool(point::PointPtr)> is_vertex);
        void DetectEdges(std::optional<VertexId> vertex_id = std::nullopt);

        const std::unordered_map<VertexId, VertexPtr>& GetVertexes() const;
        const std::unordered_map<EdgeId, EdgePtr>& GetEdges() const;
    private:
        matrix::GraphRecognitionMatrix grm_;
        std::unordered_map<VertexId, VertexPtr> vertexes_;
        std::unordered_map<EdgeId, EdgePtr> edges_;

    private:
        void DetectPortPoints();
    };
}
