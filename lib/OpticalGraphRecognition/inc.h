#pragma once

#include <opencv2/opencv.hpp>
#include <cassert>

namespace std {
    template <>
    class hash<cv::Point> {
    public:
        uint64_t operator()(const cv::Point& point) const {
            auto hashFn = std::hash<int>{};
            uint64_t hash1 = hashFn(point.x);
            uint64_t hash2 = hashFn(point.y);
            return hash1 ^ hash2;
        }
    };
}
