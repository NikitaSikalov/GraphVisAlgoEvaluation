#include "aesthetic_metrics.h"

namespace NOgr {
    void AestheticMetrics::PreprocessMetrics() {
        auto points_iterator = mat_.GetIterator();
        while (!points_iterator.IsEnd()) {
            const cv::Point current = points_iterator.Next();
            PreprocessPoint(current);
        }
    }

    void AestheticMetrics::PreprocessPoint(const cv::Point& point) {
        const PixelType ptype = mat_.GetPixelType(point);
        if (ptype == PixelType::VERTEX) {
            PreprocessVertexPoint(point);
        } else if (ptype == PixelType::CROSSING) {
            edge_crossings_.push_back(point);
        }
    }

    void AestheticMetrics::PreprocessVertexPoint(const cv::Point& point) {
        for (const cv::Point& neighbour : mat_.Get4Neighbourhood(point)) {
            if (vertexes_mapping_.contains(neighbour)) {
                vertexes_mapping_[point] = vertexes_mapping_[neighbour];
                return;
            }
        }
        vertexes_mapping_[point] = vertexes_cnt_++;
    }

    size_t AestheticMetrics::GetVertexesCount() {
        return vertexes_cnt_;
    }
}
