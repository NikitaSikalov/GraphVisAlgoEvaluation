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

    const cv::Vec3b OgrMat::kColorsPalette[] = {
            // {Blue, Green, Red}
            cv::Vec3b{0, 0, 0},         // EMPTY
            cv::Vec3b{255, 255, 255},   // UNKNOWN
            cv::Vec3b{255, 0, 0},       // VERTEX
            cv::Vec3b{0, 255, 0},       // EDGE
            cv::Vec3b{0, 0, 255}        // CROSSING
    };
}