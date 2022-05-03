#pragma once

#include <ogr_components/matrix.h>
#include <utils/disjoint_set.h>

#include <unordered_map>
#include <vector>
#include <memory>


namespace ogr::algo {
    template <typename Neighbourhood>
    class PointsGluer {
        using Set = utils::DisjointSet;
        using SetId = uint64_t;
        using SetPtr = std::unique_ptr<Set>;

    public:
        explicit PointsGluer(matrix::Grm& grm) : grm_(grm) {}

        void AddPoint(point::PointPtr point) {
            static SetId set_counter = 0;

            std::vector<Set*> sets;
            for (const point::PointPtr neighbour : ngh_(point, grm_)) {
                if (points_.contains(neighbour.get())) {
                    sets.push_back(points_[neighbour.get()].get());
                }
            }

            SetPtr new_set = std::make_unique<Set>(set_counter++);
            if (!sets.empty()) {
                sets.push_back(new_set.get());
                utils::MergeDisjointSets(sets);
            }

            points_[point.get()] = std::move(new_set);
        }

        uint64_t GetGroupId(point::PointPtr point) {
            if (!ContainsPoint(point)) {
                throw std::runtime_error{"Gluer does not contain point"};
            }

            SetId set_id = points_[point.get()]->GetId();
            if (!ids_.contains(set_id)) {
                ids_[set_id] = group_id_counter_++;
            }

            return ids_[set_id];
        }

        bool ContainsPoint(point::PointPtr point) const {
            return points_.contains(point.get());
        }

    private:
        matrix::Grm& grm_;
        Neighbourhood ngh_;
        std::unordered_map<point::Point*, SetPtr> points_;

        std::unordered_map<SetId, uint64_t> ids_;
        uint64_t group_id_counter_{0};
    };
}