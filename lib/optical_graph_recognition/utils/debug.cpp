#include "debug.h"

#include <crawler/step.h>
#include <crawler/step_tree_node.h>
#include <crawler/edge_crawler.h>
#include <utils/geometry.h>
#include <utils/opencv_utils.h>

#include <thread_pool.hpp>

#include <filesystem>


namespace ogr::debug {
    std::string DevDirPath;

    void DebugDump(const matrix::Grm& grm, std::optional<VertexId> vertex_filter) {
        static size_t seq_id = 0;
        static thread_pool tp;

        if (DevDirPath.empty()) {
            return;
        }

        const std::filesystem::path dev_dir(DevDirPath);
        if (!seq_id) {
            std::filesystem::remove_all(dev_dir);
            std::filesystem::create_directories(dev_dir);
        }
        seq_id++;

        const std::filesystem::path output = dev_dir / (std::to_string(seq_id) + ".png");

        cv::Mat image = vertex_filter.has_value()
                ? opencv::Grm2CvMat(grm, utils::EdgePointFilterWithSourceVertex{*vertex_filter})
                : opencv::Grm2CvMat(grm);

        tp.push_task([=]() {
            cv::imwrite(output, image);
        });
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

        ss << "IsStable = " << step_tree_node.IsStable() << "; ";
        ss << "IsValid = " << step_tree_node.IsValid() << "; ";
        ss << "StateAngle = " << static_cast<int>(step_tree_node.GetStateAngle()) << "; ";
        if (step_tree_node.GetStep()->Size() >= 2) {
            ss << "StepAngle = " << static_cast<int>(step_tree_node.GetLastStepAngle()) << "; ";
        }

        ss << "DiffStates = " << static_cast<int>(step_tree_node.GetDiffAngleWithPrevState()) << "; ";
        ss << "DiffStables = " << static_cast<int>(step_tree_node.GetDiffAngleWithPrevStableState()) << "; ";
        if (step_tree_node.GetStep()->Size() >= 2) {
            ss << "DiffStep = " << static_cast<int>(step_tree_node.GetDiffAngleWithLastStep()) << "; ";
        }

        ss << "Depth = " << step_tree_node.GetDepth() << "; ";
        ss << "SDepth = " << step_tree_node.GetStableDepth() << "; ";

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
