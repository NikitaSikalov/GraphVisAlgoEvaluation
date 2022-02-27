#pragma once

#include "extended_mat.h"
#include "disjoint_set.h"

#include <unordered_map>
#include <vector>
#include <memory>

namespace NOgr {
    struct Stats {
        double mean;
        double min;
        double max;
        double variance;
    };

    class AestheticMetrics {
    public:
        using DisjointSetPtr = std::unique_ptr<NUtils::DisjointSet>;
    public:
        AestheticMetrics(OgrMat mat) : mat_(std::move(mat)) {};

        void PreprocessMetrics();

        size_t GetVertexesCount() const;
        size_t GetEdgeCrossingsCount() const;
        size_t GetEdgesCount() const;

        double GetIncClutterValue();
        Stats GetEdgeLengthsStats();
        double GetAmbiguityValue();

    private:
        OgrMat mat_;

    private:
        std::unordered_map<cv::Point, DisjointSetPtr> vertexes_mapping_;
        std::vector<cv::Point> edge_crossings_;
        size_t vertexes_id_counter_{0};
        size_t not_empty_points_{0};

    private:
        void PreprocessPoint(const cv::Point&);
        void PreprocessVertexPoint(const cv::Point&);
    };
}
