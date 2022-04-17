#pragma once

#include <ogr_components/matrix.h>
#include <utils/point_filters.h>

#include <opencv2/opencv.hpp>

namespace ogr::opencv {
    cv::Mat GetThinningImage(const cv::Mat& image);
    double ColorDistance(const cv::Vec3b& pixel1, const cv::Vec3b& pixel2);
    cv::Mat Grm2CvMat(const matrix::Grm& grm, const utils::IPointFilter& point_filter = utils::IdentityPointFilter{});
}