#include "aesthetic_metrics.h"

#include <unordered_set>

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
        if (ptype == PixelType::EMPTY) {
            return;
        } else if (ptype == PixelType::VERTEX) {
            PreprocessVertexPoint(point);
        } else if (ptype == PixelType::CROSSING) {
            edge_crossings_.push_back(point);
        }
        not_empty_points_++;
    }

    void AestheticMetrics::PreprocessVertexPoint(const cv::Point& point) {
        StackVector<NUtils::DisjointSet*, 9> sets;
        for (const cv::Point& neighbour : mat_.Get8Neighbourhood(point)) {
            if (vertexes_mapping_.contains(neighbour)) {
                sets.PushBack(vertexes_mapping_[neighbour].get());
            }
        }

        DisjointSetPtr new_set = std::make_unique<NUtils::DisjointSet>(vertexes_id_counter_++);

        if (!sets.Empty()) {
            sets.PushBack(new_set.get());
            NUtils::MergeSets(sets);
        }

        vertexes_mapping_[point] = std::move(new_set);
    }

    size_t AestheticMetrics::GetVertexesCount() const {
        std::unordered_set<size_t> vertexes_ids;
        for (const auto& [_, set] : vertexes_mapping_) {
            vertexes_ids.insert(set->GetId());
        }
        return vertexes_ids.size();
    }

    size_t AestheticMetrics::GetEdgeCrossingsCount() const {
        return edge_crossings_.size();
    }
}
