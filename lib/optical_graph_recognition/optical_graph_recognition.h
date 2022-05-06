#pragma once

#include <ogr_components/matrix.h>
#include <iterators/consecutive_iterator.h>
#include <vertex/detectors.h>
#include <utils/debug.h>
#include <map/composite_map.h>

#include <opencv2/opencv.hpp>
#include <tabulate/table.hpp>

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

        void UpdateIncUsage(const cv::Mat& source_image);

        void DetectVertexes(std::function<bool(point::PointPtr)> is_vertex);
        void DetectEdges(std::optional<VertexId> vertex_id = std::nullopt);

        void UnionFoundEdges();
        void IntersectFoundEdges();
        void BuildEdgeBundlingMap();
        void MarkCrossingsPoints();

        void DumpResultImages(const std::filesystem::path& output_dir, std::optional<VertexId> vertex = std::nullopt);
        tabulate::Table GetGeneralData(const std::string& title);
        tabulate::Table GetExtendedGeneralData(const std::string& title, OpticalGraphRecognition& baseline);
        tabulate::Table GetEdgesInfo(const std::string& title, OpticalGraphRecognition& baseline);

    private:
        matrix::GraphRecognitionMatrix grm_;
        std::unordered_map<VertexId, VertexPtr> vertexes_;
        std::unordered_map<EdgeId, EdgePtr> edges_;
        std::unordered_map<uint64_t, std::vector<point::PointPtr>> crossing_areas_;

        map::CompositeMap<bool, EdgeId, EdgeId> bundling_map_;
        map::CompositeMap<size_t, EdgeId, EdgeId> edge_lengths_;
        map::CompositeMap<EdgeId, VertexId, VertexId> adjacency_map_;

        size_t inc_usage_;

    private:
        void DetectPortPoints();
        void PostProcessEdges(bool intersect);
        void ClearGrmFromUnusedEdgePoints();
        void CalculateEdgesLength();
        bool IsCrossingPoint(const point::EdgePointPtr&);
        std::vector<EdgeId> GetEdgesIds();

    private:
        void ProcessSingleEdgeLength(EdgePtr edge);

    private:
        static EdgePtr ChooseBestEdge(EdgePtr e1, EdgePtr e2);
    };
}
