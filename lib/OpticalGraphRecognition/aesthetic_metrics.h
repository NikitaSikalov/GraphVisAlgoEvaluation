#pragma once

#include "extended_mat.h"

#include <unordered_map>
#include <vector>

namespace NOgr {
    struct Stats {
        double mean;
        double min;
        double max;
        double variance;
    };

    class AestheticMetrics {
    public:
        AestheticMetrics(OgrMat mat) : mat_(std::move(mat)) {};

        void PreprocessMetrics();

        size_t GetVertexesCount();
        size_t GetEdgeCrossingsCount();
        size_t GetEdgesCount();

        double GetIncClutter();
        Stats GetEdgeLengthsStats();
        double GetAmbiguityValue();

    private:
        OgrMat mat_;

    private:
        std::unordered_map<cv::Point, size_t> vertexes_mapping_;
        std::vector<cv::Point> edge_crossings_;
        size_t vertexes_cnt_{0};

    private:
        void PreprocessPoint(const cv::Point&);
        void PreprocessVertexPoint(const cv::Point&);
    };
}
