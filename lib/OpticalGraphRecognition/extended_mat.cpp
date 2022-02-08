#include "extended_mat.h"

namespace ogr {
    PixelType OgrMat::GetPixelType(const cv::Point &point) const {
        const uchar type = cv::Mat::at<uchar>(point);
        assert(type < static_cast<uchar>(PixelType::MAX));

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
}