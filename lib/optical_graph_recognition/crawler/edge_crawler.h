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
        virtual void Materialize(EdgeId edge_id) && = 0;
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
        void Materialize(EdgeId edge_id) && override;
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
        return fabs(path_position_->GetDiffAngleWithPrevState()) <= angle_diff_threshold;
    }

    template <size_t StepMaxSize, size_t SubPathStepsSize>
    inline void EdgeCrawler<StepMaxSize, SubPathStepsSize>::Materialize(EdgeId edge_id) && {
        // TODO: to be implemented
        throw std::runtime_error{"Materialize not implemented"};
    }
}