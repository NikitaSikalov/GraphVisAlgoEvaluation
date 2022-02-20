#pragma once

#include "graph_crawler.h"
#include "extended_mat.h"
#include "inc.h"

#include <queue>
#include <unordered_set>

namespace NOgr {
    class PixelClassifier {
    public:
        explicit PixelClassifier(const cv::Mat& graph_image);

        void FindVertexes(const cv::Vec3b &vertex_color = {0, 0, 0}, double threshold = 80.0);
        void ClassifyEdgePixels();
        [[nodiscard]] const OgrMat& GetClassifiedImage() const;

    private:
        void TryAddCrawler();
        void RunCrawler(GraphCrawler* crawler);
        void ReduceCrossingPoints();
        void LookAroundCrossing(const cv::Point& crossing);

    private:
        const cv::Mat source_image_;
        OgrMat classified_image_;

        std::queue<GraphCrawler> crawlers_;
        std::unordered_set<cv::Point> crossings_;
        std::unordered_set<cv::Point> considered_;
    };
}
