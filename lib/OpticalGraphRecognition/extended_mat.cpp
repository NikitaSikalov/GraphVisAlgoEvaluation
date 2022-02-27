#include "extended_mat.h"

namespace NOgr {
    PixelType OgrMat::GetPixelType(const cv::Point &point) const {
        const uint8_t type = this->at<uint8_t>(point);
        assert(type < static_cast<uint8_t>(PixelType::MAX));

        return static_cast<PixelType>(type);
    }

    auto GetNotPixelTypeFilter(PixelType type) {
        return [type](const OgrMat& image, const cv::Point& point) {
            return image.GetPixelType(point) != type;
        };
    }

    StackVector<cv::Point, 4> OgrMat::Get4Neighbourhood(const cv::Point& point) const {
        const StackVector<cv::Point, 4> neighbours{
                cv::Point{point.x - 1, point.y},
                cv::Point{point.x + 1, point.y},
                cv::Point{point.x, point.y + 1},
                cv::Point{point.x, point.y - 1}
        };

        return FilterPoints(neighbours, NOT(PixelType::EMPTY));
    }

    StackVector<cv::Point, 8> OgrMat::Get8Neighbourhood(const cv::Point &point) const {
        const StackVector<cv::Point, 8> neighbours{
            cv::Point{point.x - 1, point.y},
            cv::Point{point.x, point.y - 1},
            cv::Point{point.x, point.y + 1},
            cv::Point{point.x + 1, point.y},
            cv::Point{point.x - 1, point.y - 1},
            cv::Point{point.x + 1, point.y + 1},
            cv::Point{point.x + 1, point.y - 1},
            cv::Point{point.x - 1, point.y + 1}
        };

        return FilterPoints(neighbours, NOT(PixelType::EMPTY));
    }

    OgrMat OgrMat::MakeOgrMatFromBinary(const cv::Mat &binary_image) {
        OgrMat result(binary_image.size(), CV_8U);
        for (int row = 0; row < binary_image.rows; ++row) {
            for (int col = 0; col < binary_image.cols; ++col) {
                const cv::Point point{col, row};
                if (binary_image.at<uint8_t>(point)) {
                    result.at<uint8_t>(point) = static_cast<uint8_t>(PixelType::UNKNOWN);
                } else {
                    result.at<uint8_t>(point) = static_cast<uint8_t>(PixelType::EMPTY);
                }
            }
        }

        return result;
    }

    void OgrMat::SetPixelType(const cv::Point& point, PixelType pixel_type) {
        this->at<uint8_t>(point) = static_cast<uint8_t>(pixel_type);
    }

    cv::Mat OgrMat::GetColoredImage() const {
        cv::Mat result(this->size(), CV_8UC3);
        for (int row = 0; row < this->rows; ++row) {
            for (int col = 0; col < this->cols; ++col) {
                const cv::Point point(col, row);
                const auto color_index = static_cast<uint8_t>(GetPixelType(point));
                result.at<cv::Vec3b>(point) = kColorsPalette[color_index];
            }
        }

        return result;
    }

    OgrMat::Iterator::Iterator(const OgrMat &mat) : mat_(mat) {
        for (int row = 0; row < mat_.rows; ++row) {
            for (int col = 0; col < mat_.cols; ++col) {
                const cv::Point p{col, row};
                if (mat_.GetPixelType(p) == PixelType::VERTEX) {
                    start_points_.insert(p);
                }
            }
        }
        InsertStartPoint();
    }

    cv::Point OgrMat::Iterator::Next() {
        assert(!IsEnd());

        cv::Point result;
        while (true) {
            if (bfs_queue_.empty() && !start_points_.empty()) {
                InsertStartPoint();
            }

            const cv::Point next = bfs_queue_.front();
            bfs_queue_.pop();

            const auto neighbours = mat_.Get8Neighbourhood(next);
            if (neighbours.Contains(prev_) || next == prev_) {
                result = next;
                break;
            }

            start_points_.insert(next);
            used_.erase(next);
        }

        const auto next_neighbours = mat_.Get8Neighbourhood(result);
        assert(next_neighbours.Contains(prev_) || result == prev_);
        for (const cv::Point &p : next_neighbours) {
            if (used_.contains(p)) {
                continue;
            }

            start_points_.erase(p);
            bfs_queue_.push(p);
            used_.insert(p);
        }

        prev_ = result;
        return result;
    }

    bool OgrMat::Iterator::IsEnd() const {
        return bfs_queue_.empty() && start_points_.empty();
    }

    void OgrMat::Iterator::InsertStartPoint() {
        assert(!start_points_.empty());
        assert(bfs_queue_.empty());

        const cv::Point point = *start_points_.begin();
        LOG_DEBUG << "Insert new start point to BFS queue " << point;

        assert(!used_.contains(point));
        bfs_queue_.push(point);
        prev_ = point;
        start_points_.erase(point);
        used_.insert(point);
    }

    const cv::Vec3b OgrMat::kColorsPalette[] = {
            // {Blue, Green, Red}
            cv::Vec3b{0, 0, 0},         // EMPTY
            cv::Vec3b{255, 255, 255},   // UNKNOWN
            cv::Vec3b{255, 0, 0},       // VERTEX
            cv::Vec3b{0, 255, 0},       // EDGE
            cv::Vec3b{0, 0, 255}        // CROSSING
    };
}