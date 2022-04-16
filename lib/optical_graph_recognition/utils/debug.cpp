#include "debug.h"

#include <crawler/step.h>
#include <crawler/step_tree_node.h>
#include <crawler/edge_crawler.h>
#include <utils/geometry.h>

#include <filesystem>

namespace ogr::debug {
    namespace {
        bool IsPointOfEdgeWithVertexSource(point::PointPtr point, VertexId source_id) {
            if (!point::IsEdgePoint(point)) {
                return false;
            }

            point::EdgePointPtr edge_point = std::dynamic_pointer_cast<point::EdgePoint>(point);
            for (const std::weak_ptr<Edge> edge : edge_point->edges) {
                if (edge.lock()->v1 == source_id) {
                    return true;
                }
            }

            return false;
        }

        bool ContainsVertexPointInNeighbourhood(point::PointPtr point, const matrix::Grm& grm) {
            iterator::Neighbourhood4 neighbourhood8;
            for (point::PointPtr point : neighbourhood8(point, grm)) {
                if (point::IsVertexPoint(point)) {
                    return true;
                }
            }

            return false;
        }
    }


    std::string DevDirPath;

    cv::Mat DebugDumpGrm2CvMat(const matrix::Grm& grm, std::optional<VertexId> vertex_filter) {
        const size_t rows = matrix::Rows(grm);
        const size_t cols = matrix::Columns(grm);
        cv::Mat cv_image(rows, cols, CV_8UC3);

        // {B, G, R}
        const cv::Vec3b filled_point_color{255, 255, 255};
        const cv::Vec3b vertex_point_color{0, 0, 255};
        const cv::Vec3b marked_point_color{33, 111, 255};
        const cv::Vec3b edge_point_color{63, 253, 255};

        for (size_t row = 0; row < matrix::Rows(grm); ++row) {
            for (size_t col = 0; col < matrix::Columns(grm); ++col) {
                const cv::Point cv_point(col, row);
                const point::PointPtr& grm_point = grm[row][col];
                if (point::IsVertexPoint(grm_point)) {
                    cv_image.at<cv::Vec3b>(cv_point) = vertex_point_color;
                } else if (vertex_filter.has_value() && IsPointOfEdgeWithVertexSource(grm_point, vertex_filter.value())) {
                    cv_image.at<cv::Vec3b>(cv_point) = edge_point_color;
                } else if (!vertex_filter.has_value() && point::IsEdgePoint(grm_point)) {
                    cv_image.at<cv::Vec3b>(cv_point) = edge_point_color;
                } else if (!grm_point->IsEmpty() && point::IsMarkedPoint(grm_point)) {
                    cv_image.at<cv::Vec3b>(cv_point) = marked_point_color;
                } else if (!grm_point->IsEmpty()) {
                    cv_image.at<cv::Vec3b>(cv_point) = filled_point_color;
                } else if (ContainsVertexPointInNeighbourhood(grm_point, grm)) {
                    cv_image.at<cv::Vec3b>(cv_point) = vertex_point_color;
                }
            }
        }

        return cv_image;
    }

    void DebugDump(const matrix::Grm& grm, const bool force, std::optional<VertexId> vertex_filter) {
        static size_t seq_id = 0;
        constexpr size_t kFrequency = 1;

        const std::filesystem::path dev_dir(DevDirPath);
        if (!seq_id) {
            std::filesystem::remove_all(dev_dir);
            std::filesystem::create_directories(dev_dir);
        }

        seq_id++;
        if (seq_id % kFrequency != 0 && !force) {
            return;
        }

        const std::filesystem::path output = dev_dir / (std::to_string(seq_id) + ".png");
        cv::Mat image = DebugDumpGrm2CvMat(grm, vertex_filter);

        cv::imwrite(output, image);
    }

    std::string DebugDump(const point::Point& point) {
        std::stringstream ss;
        ss << "(" << point.column << ", " << point.row << ")";

        return ss.str();
    }

    std::string DebugDump(const Vertex& vertex) {
        std::stringstream ss;
        ss << "Vertex info: ";
        ss << "Id = " << vertex.id << "; ";
        ss << "Points: [";
        for (const point::WeakPointPtr point : vertex.points) {
            ss << DebugDump(*point.lock()) << " ";
        }
        ss << "]; ";

        ss << "Port points: [";
        for (const point::WeakPointPtr point : vertex.port_points) {
            ss << DebugDump(*point.lock()) << " ";
        }
        ss << "];";

        return ss.str();
    }

    std::string DebugDump(const crawler::IStep& step) {
        std::stringstream ss;
        ss << "Step info: ";
        ss << "Size = " << step.Size() << "; ";

        if (step.Size() >= 2) {
            ss << "Angle = " << step.GetDirectionAngle() << "; ";
        }

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
        ss << "Point = " << DebugDump(*(step_tree_node.GetStep()->Back())) << "; ";

        if (step_tree_node.GetStep()->Size() >= 2) {
            ss << "Step angle = " << static_cast<int>(step_tree_node.GetLastStepAngle()) << "; ";
        }

        ss << "State angle = " << static_cast<int>(step_tree_node.GetStateAngle()) << "; ";
        ss << "Diff angle = " << static_cast<int>(step_tree_node.GetDiffAngleWithPrevState()) << "; ";
        ss << "Depth = " << step_tree_node.GetDepth() << "; ";

        return ss.str();
    }

    std::string DebugDump(const crawler::IEdgeCrawler& crawler) {
        std::stringstream ss;
        ss << "Crawler info: ";
        ss << "Current step tree node info: ";
        ss << DebugDump(*crawler.GetCurrentStepTreeNode());

        return ss.str();
    }
}
