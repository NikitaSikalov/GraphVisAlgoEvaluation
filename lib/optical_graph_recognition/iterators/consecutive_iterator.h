#pragma once

#include <queue>
#include <optional>

#include <ogr_components/matrix.h>
#include <iterators/neighbours.h>

namespace ogr::iterator {
    template <typename NeighboursStrategy>
    class ConsecutivePointsIterator {
    public:
        ConsecutivePointsIterator(const matrix::Grm &grm, point::PointPtr start_point) : grm_(grm) {
            bfs_queue_.push(start_point);
        }

        /**
         * Return next point in neighbourhood of previous returned point
         * */
        std::optional<point::PointPtr> Next() {
            while (true) {
                if (bfs_queue_.empty()) {
                    return std::nullopt;
                }

                const point::PointPtr point = bfs_queue_.front();
                bfs_queue_.pop();

                const auto neighbours = neighbourhood_(point, grm_);

                // Check that previous considered point is in neighbourhood of new point
                if (prev_point_.get() != nullptr && !ContainsPoint(neighbours, prev_point_)) {
                    start_points_.push_back(point);
                    continue;
                }

                // Add unmarked neighbours to BFS queue
                const auto unmarked_neighbours = filter::FilterMarkedPoints(neighbours);
                for (const point::PointPtr& neighbour : unmarked_neighbours) {
                    bfs_queue_.push(neighbour);
                }

                prev_point_ = point;
                return point;
            }
        }

        /**
         * Generate next ConsecutivePointsIterator from unprocessed start points
         * */
        std::optional<ConsecutivePointsIterator> GetAnotherPathIterator() && {
            if (start_points_.empty()) {
                return std::nullopt;
            }

            ConsecutivePointsIterator next_iterator(grm_, start_points_.back());
            start_points_.pop_back();
            next_iterator.start_points_ = std::move(start_points_);

            return next_iterator;
        }

    private:
        const matrix::Grm& grm_;
        point::PointPtr prev_point_{nullptr};
        std::vector<point::PointPtr> start_points_;

        std::queue<point::PointPtr> bfs_queue_;
        NeighboursStrategy neighbourhood_;
    };
}