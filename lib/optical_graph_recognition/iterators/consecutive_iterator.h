#pragma once

#include <ogr_components/matrix.h>
#include <iterators/neighbours.h>
#include <utils/debug.h>

#include <plog/Log.h>

#include <queue>
#include <optional>
#include <utility>
#include <unordered_set>
#include <algorithm>

namespace ogr::iterator {
    template <typename NeighboursStrategy>
    class ConsecutivePointsIterator {
        using Path = std::vector<point::PointPtr>;
    public:
        ConsecutivePointsIterator(const matrix::Grm &grm, point::PointPtr start_point) : grm_(grm) {
            used_.insert(start_point.get());
            bfs_queue_.push(std::make_pair(start_point, std::vector<point::PointPtr>{}));
        }

        /**
         * Return next point in neighbourhood of previous returned point
         * */
        std::optional<point::PointPtr> Next() {
            while (true) {
                if (!buffer_.empty()) {
                    auto point = buffer_.back();
                    buffer_.pop_back();
                    prev_point_ = point;
                    return point;
                }

                if (bfs_queue_.empty()) {
                    return std::nullopt;
                }

                auto& queue_top = bfs_queue_.front();
                const point::PointPtr point = queue_top.first;
                std::vector<point::PointPtr> subpath = std::move(queue_top.second);
                bfs_queue_.pop();

                LOG_DEBUG << "Next iteration candidate " << debug::DebugDump(*point);

                const auto neighbours = neighbourhood_(point, grm_);

                // Check that previous considered point is in neighbourhood of new point
                if (prev_point_.get() != nullptr && !ContainsPoint(neighbours, prev_point_)) {
                    other_paths_.emplace_back(std::make_pair(point, subpath));
                    LOG_DEBUG << "Skip point";
                    continue;
                }

                subpath.push_back(point);

                // Add unmarked neighbours to BFS queue
                const auto unmarked_neighbours = filter::FilterMarkedPoints(neighbours);
                for (const point::PointPtr& neighbour : unmarked_neighbours) {
                    if (used_.contains(neighbour.get())) {
                        continue;
                    }
                    LOG_DEBUG << "Push to bfs queue point: " << debug::DebugDump(*neighbour);
                    bfs_queue_.push(std::make_pair(neighbour, subpath));
                    used_.insert(neighbour.get());
                }

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
                std::reverse(buffer_.begin(), buffer_.end());

                std::queue<std::pair<point::PointPtr, Path>> next_queue;
                next_queue.push(std::make_pair(next_start_point, subpath));
                bfs_queue_ = std::move(next_queue);
                used_.clear();

                return true;
            }
        }

    private:
        // Source grm matrix
        const matrix::Grm& grm_;

        // Pair {path to this point, start point}
        std::vector<std::pair<point::PointPtr, Path>> other_paths_;

        // Buffer of some points that should be iterated of firstly
        Path buffer_;

        // Implementation details fields
        point::PointPtr prev_point_{nullptr};
        std::queue<std::pair<point::PointPtr, Path>> bfs_queue_;
        NeighboursStrategy neighbourhood_;
        std::unordered_set<point::Point*> used_;
    };
}