#include "pixel_classifier.h"
#include "utils.h"

#include <plog/Log.h>

namespace NOgr {
    PixelClassifier::PixelClassifier(const cv::Mat& graph_image) : source_image_(graph_image) {
        cv::Mat grayscale_image;
        cv::cvtColor(graph_image, grayscale_image, cv::COLOR_BGR2GRAY);

        const cv::Mat thinning_image = GetThinningImage(grayscale_image);
        classified_image_ = OgrMat::MakeOgrMatFromBinary(thinning_image);
    }

    void PixelClassifier::FindVertexes(const cv::Vec3b& vertex_color, double threshold) {
        for (int row = 0; row < source_image_.rows; ++row) {
            for (int col = 0; col < source_image_.cols; ++col) {
                const cv::Point point{col, row};
                if (classified_image_.GetPixelType(point) == PixelType::EMPTY) {
                    continue;
                }

                const cv::Vec3b source_color = source_image_.at<cv::Vec3b>(point);
                if (ColorDistance(source_color, vertex_color) <= threshold) {
                    classified_image_.SetPixelType(point, PixelType::VERTEX);
                }
            }
        }
    }

    void PixelClassifier::ClassifyEdgePixels() {
        TryAddCrawler();

        while (!crawlers_.empty()) {
            while (!crawlers_.empty()) {
                RunCrawler(&crawlers_.front());
                crawlers_.pop();
            }

            TryAddCrawler();
        }

        ReduceCrossingPoints();
    }

    void PixelClassifier::ReduceCrossingPoints() {
        std::unordered_set<cv::Point> crossingsCopy = crossings_;
        for (const cv::Point& crossing : crossingsCopy) {
            classified_image_.SetPixelType(crossing, PixelType::EDGE);
            if (!crossings_.contains(crossing)) {
                continue;
            }

            LookAroundCrossing(crossing);
        }

        for (const cv::Point& crossing : crossings_) {
            classified_image_.SetPixelType(crossing, PixelType::CROSSING);
        }
    }

    void PixelClassifier::TryAddCrawler() {
        for (int row = 0; row < source_image_.rows; ++row) {
            for (int col = 0; col < source_image_.cols; ++col) {
                const cv::Point point(col, row);
                if (classified_image_.GetPixelType(point) == PixelType::VERTEX) {
                    for (const cv::Point& neighbour : classified_image_.Get8Neighbourhood(point)) {
                        if (classified_image_.GetPixelType(neighbour) == PixelType::UNKNOWN) {
                            crawlers_.emplace(classified_image_, point, &considered_);
                            LOG_DEBUG << "Add new crawler from point " << point;

                            return;
                        }
                    }
                }
            }
        }
    }

    void PixelClassifier::RunCrawler(GraphCrawler *crawler) {
        while (!crawler->Empty()) {
            const auto result = crawler->Step();
            if (result.has_value()) {
                const cv::Point& crossing_point = result.value();
                crossings_.insert(result.value());
                crawlers_.emplace(classified_image_, crossing_point, &considered_);
            }
        }
    }

    void PixelClassifier::LookAroundCrossing(const cv::Point &crossing) {
        StackVector<cv::Point, 8> local_crossings;
        crossings_.erase(crossing);
        for (const cv::Point& neighbour : classified_image_.Get8Neighbourhood(crossing)) {
            if (classified_image_.GetPixelType(neighbour) == PixelType::CROSSING) {
                crossings_.erase(neighbour);
                local_crossings.PushBack(neighbour);
            }
        }

        cv::Point point_with_max_neighbours = crossing;
        size_t max_neighbours_cnt = 0;
        for (const cv::Point& crossingPoint : local_crossings) {
            for (const cv::Point& neighbour : classified_image_.Get8Neighbourhood(crossing)) {
                const auto neighbours = classified_image_.Get8Neighbourhood(neighbour);
                if (neighbours.Size() > max_neighbours_cnt) {
                    max_neighbours_cnt = neighbours.Size();
                    point_with_max_neighbours = neighbour;
                }
            }
        }

        crossings_.insert(point_with_max_neighbours);
    }

    const OgrMat & PixelClassifier::GetClassifiedImage() const {
        return classified_image_;
    }
}
