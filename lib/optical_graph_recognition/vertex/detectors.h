#pragma once

#include <ogr_components/point.h>
#include <utils/opencv_utils.h>

namespace ogr::vertex {
    class VertexPointsDetectorByColor {
        static constexpr double kDefaultThreshold = 80.0;
    public:
        VertexPointsDetectorByColor(const cv::Mat &source_colored_image, const cv::Vec3b &color,
                                    const double threshold = kDefaultThreshold)
                : source_image_(source_colored_image), sample_color_(color), threshold_(threshold) {}

        bool operator()(const point::PointPtr &point) const {
            const int row = static_cast<int>(point->row);
            const int column = static_cast<int>(point->column);

            const cv::Point cv_point(column, row);
            const cv::Vec3b point_color = source_image_.at<cv::Vec3b>(cv_point);
            if (opencv::ColorDistance(sample_color_, point_color) < threshold_) {
                return true;
            }

            return false;
        }

    private:
        const cv::Mat &source_image_;
        cv::Vec3b sample_color_;
        const double threshold_{kDefaultThreshold};
    };
}