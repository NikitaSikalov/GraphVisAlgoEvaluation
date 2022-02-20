#pragma once

#include "extended_mat.h"
#include "inc.h"

#include <queue>
#include <optional>
#include <unordered_set>

namespace NOgr {
    class GraphCrawler {
    public:
        GraphCrawler(OgrMat &source, const cv::Point &start, std::unordered_set<cv::Point> *considered)
        : source_(source), prev_(start), considered_(considered) {
            pixels_queue_.emplace(start);
        }

        [[nodiscard]] bool Empty() const {
            return pixels_queue_.empty();
        }

        std::optional<cv::Point> Step();

    private:
        OgrMat &source_;
        std::queue<cv::Point> pixels_queue_;

        cv::Point prev_;
        std::unordered_set<cv::Point> *considered_;
    };
}
