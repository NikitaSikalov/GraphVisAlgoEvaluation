#include "opencv_utils.h"

#include <iterators/neighbours.h>

#include <opencv2/ximgproc.hpp>


namespace ogr::opencv {
    namespace {
        template <typename Predicate, typename Neighbourhood = iterator::Neighbourhood8>
        bool ContainsPointInNeighbourhoodWithPredicate(
                point::PointPtr point,
                const matrix::Grm& grm,
                Predicate predicate,
                const utils::IPointFilter& point_filter
        ) {
            Neighbourhood ngh;
            for (point::PointPtr neighbour : ngh(point, grm)) {
                point::PointPtr filtered_neighbour = point_filter(neighbour);
                if (predicate(filtered_neighbour)) {
                    return true;
                }
            }

            return false;
        }
    }


    cv::Mat GetThinningImage(const cv::Mat& image) {
        cv::Mat binaryImage;
        cv::threshold(image, binaryImage, 250, 255, cv::THRESH_BINARY_INV);

        cv::Mat skel;
        cv::ximgproc::thinning(binaryImage, skel);

        return skel;
    }

    double ColorDistance(const cv::Vec3b& pixel1, const cv::Vec3b& pixel2) {
        double res = 0;
        constexpr size_t rgb_channels = 3;

        for (int i = 0; i < rgb_channels; ++i) {
            double x = static_cast<double>(pixel1[i]) - pixel2[i];
            res += x * x;
        }

        return sqrt(res);
    }

    cv::Mat Grm2CvMat(const matrix::Grm& grm, const utils::IPointFilter& point_filter) {
        const size_t rows = matrix::Rows(grm);
        const size_t cols = matrix::Columns(grm);
        cv::Mat cv_image(rows, cols, CV_8UC3);

        // {B, G, R}
        const cv::Vec3b filled_point_color{255, 255, 255};
        const cv::Vec3b vertex_point_color{0, 0, 255};
        const cv::Vec3b marked_point_color{33, 111, 255};
        const cv::Vec3b edge_point_color{63, 253, 255};
        const cv::Vec3b crossing_point_color{255, 133, 5};
        const cv::Vec3b empty_point_color{0, 0, 0};

        const cv::Vec3b dev_color{196, 13, 255};

        for (size_t row = 0; row < matrix::Rows(grm); ++row) {
            for (size_t col = 0; col < matrix::Columns(grm); ++col) {
                const cv::Point cv_point(col, row);
                const point::PointPtr point = point_filter(grm[row][col]);

                // For dev
                /*if (point::IsDevMarked(point)) {
                    cv_image.at<cv::Vec3b>(cv_point) = dev_color;
                } else */

                if (ContainsPointInNeighbourhoodWithPredicate(point, grm, point::IsVertexPoint, point_filter)) {
                    cv_image.at<cv::Vec3b>(cv_point) = vertex_point_color;
                } else if (ContainsPointInNeighbourhoodWithPredicate(point, grm, point::IsCrossingPoint, point_filter)) {
                    cv_image.at<cv::Vec3b>(cv_point) = crossing_point_color;
                } else if (point::IsVertexPoint(point)) {
                    cv_image.at<cv::Vec3b>(cv_point) = vertex_point_color;
                } else if (point::IsCrossingPoint(point)) {
                    cv_image.at<cv::Vec3b>(cv_point) = crossing_point_color;
                } else if (point::IsEdgePoint(point)) {
                    cv_image.at<cv::Vec3b>(cv_point) = edge_point_color;
                } else if (point::IsMarkedPoint(point)) {
                    cv_image.at<cv::Vec3b>(cv_point) = marked_point_color;
                } else if (!point->IsEmpty()) {
                    cv_image.at<cv::Vec3b>(cv_point) = filled_point_color;
                } else {
                    cv_image.at<cv::Vec3b>(cv_point) = empty_point_color;
                }
            }
        }

        return cv_image;
    }
}

