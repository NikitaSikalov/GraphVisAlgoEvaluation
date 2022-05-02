#pragma once

#include <ogr_components/matrix.h>
#include <iterators/consecutive_iterator.h>
#include <vertex/detectors.h>
#include <utils/debug.h>
#include <map/composite_map.h>

#include <opencv2/opencv.hpp>

#include <unordered_map>
#include <optional>
#include <filesystem>


namespace ogr {
    namespace map {
        // Composite key generator for bundling edges map
        template <>
        inline std::tuple<EdgeId, EdgeId> MakeCompositeKey(EdgeId key1, EdgeId key2) {
            return std::make_tuple(std::min(key1, key2), std::max(key1, key2));
        }
    }

    class OpticalGraphRecognition {
    public:
        explicit OpticalGraphRecognition(const cv::Mat& source_graph);

        void DetectVertexes(std::function<bool(point::PointPtr)> is_vertex);
        void DetectEdges(std::optional<VertexId> vertex_id = std::nullopt);

        const std::unordered_map<VertexId, VertexPtr>& GetVertexes() const;
        const std::unordered_map<EdgeId, EdgePtr>& GetEdges() const;

        void UnionFoundEdges();
        void IntersectFoundEdges();
        void BuildEdgeBundlingMap();

        void DumpResultImages(const std::filesystem::path output_dir, std::optional<VertexId> vertex = std::nullopt);

    private:
        matrix::GraphRecognitionMatrix grm_;
        std::unordered_map<VertexId, VertexPtr> vertexes_;
        std::unordered_map<EdgeId, EdgePtr> edges_;

        map::CompositeMap<bool, EdgeId, EdgeId> bundling_map_;
        map::CompositeMap<size_t, EdgeId, EdgeId> edge_lengths_;

    private:
        void DetectPortPoints();
        void PostProcessEdges(bool intersect);
        void ClearGrmFromUnusedEdgePoints();
        void CalculateEdgesLength();

    private:
        static EdgePtr ChooseBestEdge(EdgePtr e1, EdgePtr e2);
    };
}
