#pragma once

#include <ogr_components/matrix.h>
#include <iterators/neighbours.h>

#include <queue>
#include <optional>
#include <utility>

namespace ogr::iterator {
    template <typename NeighboursStrategy>
    class ConsecutivePointsIterator {
        using Path = std::vector<point::PointPtr>;
    public:
        ConsecutivePointsIterator(const matrix::Grm &grm, point::PointPtr start_point) : grm_(grm) {
            bfs_queue_.push(start_point);
        }

        /**
         * Return next point in neighbourhood of previous returned point
         * */
        std::optional<point::PointPtr> Next() {
            while (true) {
                if (!buffer_.empty()) {
                    auto point = buffer_.back();
                    buffer_.pop_back();
                    return point;
                }

                if (bfs_queue_.empty()) {
                    return std::nullopt;
                }

                const point::PointPtr point = bfs_queue_.front();
                bfs_queue_.pop();

                const auto neighbours = neighbourhood_(point, grm_);

                // Check that previous considered point is in neighbourhood of new point
                if (prev_point_.get() != nullptr && !ContainsPoint(neighbours, prev_point_)) {
                    other_paths_.emplace_back(std::make_pair(point, memory_));
                    continue;
                }

                // Add unmarked neighbours to BFS queue
                const auto unmarked_neighbours = filter::FilterMarkedPoints(neighbours);
                for (const point::PointPtr& neighbour : unmarked_neighbours) {
                    bfs_queue_.push(neighbour);
                }

                memory_.push_back(point);
                prev_point_ = point;
                return point;
            }
        }

        bool ResetToOtherPath() {
            while (true) {
                if (other_paths_.empty()) {
                    return false;
                }

                auto other_path = other_paths_.back();
                point::PointPtr next_start_point = other_path.first;
                Path subpath = std::move(other_path.second);
                other_paths_.pop_back();

                if (point::IsMarkedPoint(next_start_point)) {
                    continue;
                }

                buffer_ = std::move(subpath);
                decltype(bfs_queue_) next_queue;
                next_queue.push(next_start_point);
                bfs_queue_ = std::move(next_queue);

                return true;
            }
        }

    private:
        // Source grm matrix
        const matrix::Grm& grm_;

        // Memorized returned points
        Path memory_;

        // Pair {path to this point, start point}
        std::vector<std::pair<point::PointPtr, Path>> other_paths_;

        // Buffer of some points that should be iterated of firstly
        Path buffer_;

        // Implementation details fields
        point::PointPtr prev_point_{nullptr};
        std::queue<point::PointPtr> bfs_queue_;
        NeighboursStrategy neighbourhood_;
    };
}