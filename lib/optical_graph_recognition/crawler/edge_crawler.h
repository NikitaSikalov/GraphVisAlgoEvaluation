#pragma once

#include <crawler/step_tree_node.h>
#include <ogr_components/matrix.h>
#include <iterators/neighbours.h>

#include <vector>
#include <memory>

namespace ogr::crawler {
    struct IEdgeCrawler;
    using EdgeCrawlerPtr = std::shared_ptr<IEdgeCrawler>;

    struct IEdgeCrawler {
        virtual void Commit(StepPtr step) = 0;
        virtual std::vector<StepPtr> NextSteps() = 0;
        virtual bool CheckEdge(const double angle_diff_threshold) const = 0;
        virtual bool IsComplete() const = 0;
        virtual EdgePtr Materialize(VertexId source, EdgeId edge_id, matrix::Grm& grm) && = 0;
        virtual StepTreeNodePtr GetCurrentStepTreeNode() const = 0;
    };

    struct Comparator {
        bool operator()(const EdgeCrawlerPtr& crawler1, const EdgeCrawlerPtr& crawler2) {
            return crawler1->GetCurrentStepTreeNode()->GetDiffAngleWithLastStep() > crawler2->GetCurrentStepTreeNode()->GetDiffAngleWithLastStep();
        }
    };

    template <size_t StepMaxSize, size_t SubPathStepsSize>
    class EdgeCrawler : public IEdgeCrawler {
    public:
        EdgeCrawler(matrix::Grm& grm, StepTreeNodePtr path_position);
    public:
        void Commit(StepPtr step) override;
        std::vector<StepPtr> NextSteps() override;
        bool CheckEdge(const double angle_diff_threshold) const override;
        bool IsComplete() const override;
        EdgePtr Materialize(VertexId source, EdgeId edge_id, matrix::Grm& grm) && override;
        StepTreeNodePtr GetCurrentStepTreeNode() const override;

    private:
        matrix::Grm& grm_;
        StepTreeNodePtr path_position_;
    };

    //////////////////////////////////////////////////////////////////////
    /** Implementation details */

    template <size_t StepMaxSize, size_t SubPathStepsSize>
    inline EdgeCrawler<StepMaxSize, SubPathStepsSize>::EdgeCrawler(
            matrix::Grm &grm,
            StepTreeNodePtr path_position
    ) : grm_(grm), path_position_(path_position) {

    }

    template <size_t StepMaxSize, size_t SubPathStepsSize>
    inline std::vector<StepPtr> EdgeCrawler<StepMaxSize, SubPathStepsSize>::NextSteps() {
        std::vector<point::FilledPointPtr> neighbours_vector = path_position_->GetStep()->GetUnmarkedNeighbours(grm_);
        utils::StackVector<point::FilledPointPtr, StepMaxSize * 4> neighbours(neighbours_vector);
        return MakeSteps<StepMaxSize>(neighbours, grm_);
    }

    template <size_t StepMaxSize, size_t SubPathStepsSize>
    inline bool EdgeCrawler<StepMaxSize, SubPathStepsSize>::IsComplete() const {
        // Check that tree path contains more than 1 step and last step is port (contains port point)
        return path_position_->GetDepth() > 2 && path_position_->IsPort();
    }

    template <size_t StepMaxSize, size_t SubPathStepsSize>
    inline void EdgeCrawler<StepMaxSize, SubPathStepsSize>::Commit(StepPtr step) {
        StepTreeNodePtr next_node = path_position_->MakeChild(step); // std::make_shared<PathNode>(step, path_position_);
        path_position_ = next_node;
    }

    template <size_t StepMaxSize, size_t SubPathStepsSize>
    inline StepTreeNodePtr EdgeCrawler<StepMaxSize, SubPathStepsSize>::GetCurrentStepTreeNode() const {
        return path_position_;
    }

    template <size_t StepMaxSize, size_t SubPathStepsSize>
    inline bool EdgeCrawler<StepMaxSize, SubPathStepsSize>::CheckEdge(const double angle_diff_threshold) const {
        if (!path_position_->IsValid()) {
            return false;
        }

        if (path_position_->IsStable()) {
            return true;
        }

        return fabs(path_position_->GetDiffAngleWithPrevState()) <= angle_diff_threshold;
    }

    template <size_t StepMaxSize, size_t SubPathStepsSize>
    inline EdgePtr EdgeCrawler<StepMaxSize, SubPathStepsSize>::Materialize(VertexId source, EdgeId edge_id, matrix::Grm& grm) && {
        if (!path_position_->IsPort()) {
            throw std::runtime_error{"Last path position is not port: invalid materialize"};
        }

        point::VertexPointPtr port_point;
        for (point::PointPtr point : path_position_->GetStep()->GetPoints()) {
            if (point::IsPortPoint(point)) {
                port_point = std::dynamic_pointer_cast<point::VertexPoint>(point);
            }
        }

        if (!port_point) {
            throw std::runtime_error{"Port point not found"};
        }

        VertexId destination = port_point->vertex.lock()->id;
        EdgePtr edge = std::make_shared<Edge>(edge_id, source, destination);
        StepTreeNodePtr current_path_node = path_position_;

        while (!current_path_node->IsRoot()) {
            StepPtr step = current_path_node->GetStep();
            edge->irregularity = std::max(edge->irregularity, current_path_node->GetDiffAngleWithPrevStableState());

            for (point::PointPtr step_point : step->GetPoints()) {
                // Attention! Point in grm can be different from point in step!
                point::PointPtr point = grm[step_point->row][step_point->column];
                if (point::IsPortPoint(point)) {
                    continue;
                }

                point::EdgePointPtr edge_point;
                if (point::IsEdgePoint(point)) {
                    edge_point = std::dynamic_pointer_cast<point::EdgePoint>(point);
                } else {
                    edge_point = std::make_shared<point::EdgePoint>(point->row, point->column);
                    grm[point->row][point->column] = edge_point;
                }

                edge_point->Mark();
                edge_point->edges.push_back(edge);
                edge->points.push_back(edge_point);
            }

            current_path_node = current_path_node->GetParentNode();
        }

        return edge;
    }
}