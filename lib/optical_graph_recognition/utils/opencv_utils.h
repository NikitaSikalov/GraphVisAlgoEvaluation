#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/ximgproc.hpp>

namespace ogr::opencv {
    inline cv::Mat GetThinningImage(const cv::Mat& image) {
        cv::Mat binaryImage;
        cv::threshold(image, binaryImage, 250, 255, cv::THRESH_BINARY_INV);

        cv::Mat skel;
        cv::ximgproc::thinning(binaryImage, skel);

        return skel;
    }

    inline double ColorDistance(const cv::Vec3b& pixel1, const cv::Vec3b& pixel2) {
        double res = 0;
        constexpr size_t rgb_channels = 3;

        for (int i = 0; i < rgb_channels; ++i) {
            double x = static_cast<double>(pixel1[i]) - pixel2[i];
            res += x * x;
        }

        return sqrt(res);
    }
}