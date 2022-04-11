#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/ximgproc.hpp>

namespace ogr::opencv {
    inline cv::Mat GetThinningImage(const cv::Mat& image) {
        cv::Mat blurredImage;
        cv::GaussianBlur(image, blurredImage, {5, 5}, 0);

        cv::Mat binaryImage;
        cv::threshold(blurredImage, binaryImage, 250, 255, cv::THRESH_BINARY_INV);

        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_CROSS, {5, 5});
        cv::Mat preprocessedImage;
        morphologyEx(binaryImage, preprocessedImage, cv::MORPH_OPEN, kernel);

        cv::Mat skel;
        cv::ximgproc::thinning(preprocessedImage, skel);

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