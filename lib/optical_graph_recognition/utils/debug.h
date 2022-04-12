#pragma once

#include <opencv2/opencv.hpp>

#include <ogr_components/matrix.h>

namespace ogr::debug {
    inline cv::Mat DebugDumpGrm2CvMat(const matrix::Grm& grm) {
        const size_t rows = matrix::Rows(grm);
        const size_t cols = matrix::Columns(grm);
        cv::Mat cv_image(rows, cols, CV_8UC3);

        const cv::Vec3b filled_point_color{255, 255, 255};
        const cv::Vec3b vertex_point_color{0, 255, 0};
        const cv::Vec3b edge_point_color{0, 0, 255};

        for (size_t row = 0; row < matrix::Rows(grm); ++row) {
            for (size_t col = 0; col < matrix::Columns(grm); ++col) {
                const cv::Point cv_point(col, row);
                const point::PointPtr& grm_point = grm[row][col];
                if (point::IsVertexPoint(grm_point)) {
                    cv_image.at<cv::Vec3b>(cv_point) = vertex_point_color;
                } else if (point::IsEdgePoint(grm_point)) {
                    cv_image.at<cv::Vec3b>(cv_point)  = edge_point_color;
                } else if (!grm_point->IsEmpty()) {
                    cv_image.at<cv::Vec3b>(cv_point) = filled_point_color;
                }
            }
        }

        return cv_image;
    }
}