#include "graph_crawler.h"

namespace NOgr {
    std::optional<cv::Point> GraphCrawler::Step() {
        cv::Point current = pixels_queue_.front();
        pixels_queue_.pop();

        const auto neighbours8 = source_.Get8Neighbourhood(current);
        if (!neighbours8.Contains(prev_) && current != prev_) {
            source_.SetPixelType(current, PixelType::CROSSING);
            return current;
        }

        if (current != prev_) {
            source_.SetPixelType(current, PixelType::EDGE);
        }

        for (const cv::Point& neighbour : neighbours8) {
           if (considered_->contains(neighbour)) {
               continue;
           }

           if (source_.GetPixelType(neighbour) != PixelType::UNKNOWN) {
               continue;
           }

           pixels_queue_.emplace(neighbour);
           considered_->insert(neighbour);
        }

        prev_ = current;
        return std::nullopt;
    }
}
