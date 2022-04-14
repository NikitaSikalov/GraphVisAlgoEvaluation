#include "debug.h"

#include <crawler/step.h>
#include <crawler/step_tree_node.h>
#include <utils/geometry.h>

namespace ogr::debug {
    cv::Mat DebugDumpGrm2CvMat(const matrix::Grm& grm) {
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

    std::string DebugDump(const point::Point& point) {
        std::stringstream ss;
        ss << "(" << point.column << ", " << point.row << ")";

        return ss.str();
    }

    std::string DebugDump(const Vertex& vertex) {
        std::stringstream ss;
        ss << "Vertex id = " << vertex.id << "\n";
        ss << "Vertex points: ";
        for (const point::WeakPointPtr point : vertex.points) {
            ss << DebugDump(*point.lock()) << " ";
        }
        ss << "\n";

        ss << "Vertex port points: ";
        for (const point::WeakPointPtr point : vertex.port_points) {
            ss << DebugDump(*point.lock()) << " ";
        }

        return ss.str();
    }

    std::string DebugDump(const crawler::IStep& step) {
        std::stringstream ss;
        ss << "Step info: ";
        ss << "Size = " << step.Size() << "; ";
        ss << "Angle = " << step.GetDirectionAngle() << "; ";
        ss << "Start point = " << DebugDump(*step.Front()) << "; ";
        ss << "End point = " << DebugDump(*step.Back()) << "; ";
        ss << "Points = ";
        for (auto point : step.GetPoints()) {
            ss << DebugDump(*point) << " ";
        }

        return ss.str();
    }

    std::string DebugDump(const crawler::IStepTreeNode& step_tree_node) {
        std::stringstream ss;
        ss << "Step tree node info: ";
        ss << "End point = " << DebugDump(*(step_tree_node.GetStep()->Back())) << "; ";
        ss << "Last step angle = " << step_tree_node.GetLastStepAngle() << "; ";
        ss << "State angle = " << step_tree_node.GetStateAngle() << "; ";
        ss << "Diff with prev state = " << step_tree_node.GetDiffAngleWithPrevState() << "; ";

        return ss.str();
    }
}
