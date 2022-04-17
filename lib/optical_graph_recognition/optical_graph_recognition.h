#pragma once

#include <ogr_components/matrix.h>
#include <iterators/consecutive_iterator.h>
#include <vertex/detectors.h>
#include <utils/debug.h>

#include <opencv2/opencv.hpp>

#include <unordered_map>
#include <optional>
#include <filesystem>


namespace ogr {
    class OpticalGraphRecognition {
    public:
        explicit OpticalGraphRecognition(const cv::Mat& source_graph);

        void DetectVertexes(std::function<bool(point::PointPtr)> is_vertex);
        void DetectEdges(std::optional<VertexId> vertex_id = std::nullopt);

        const std::unordered_map<VertexId, VertexPtr>& GetVertexes() const;
        const std::unordered_map<EdgeId, EdgePtr>& GetEdges() const;

        void UnionFoundEdges();
        void IntersectFoundEdges();

        void DumpResultImages(const std::filesystem::path output_dir);

    private:
        matrix::GraphRecognitionMatrix grm_;
        std::unordered_map<VertexId, VertexPtr> vertexes_;
        std::unordered_map<EdgeId, EdgePtr> edges_;

    private:
        void DetectPortPoints();
        void PostProcessEdges(bool intersect);
        void ClearGrmFromUnusedEdgePoints();

    private:
        static EdgePtr ChooseBestEdge(EdgePtr e1, EdgePtr e2);
    };
}
