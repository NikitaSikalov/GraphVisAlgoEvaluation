#pragma once

#include <crawler/step_tree_node.h>
#include <ogr_components/matrix.h>
#include <iterators/neighbours.h>

#include <vector>

namespace ogr::crawler {
    template <size_t StepMaxSize, size_t SubPathStepsSize>
    class EdgeCrawler {
    public:
        EdgeCrawler(matrix::Grm& grm, StepTreeNodePtr path_position);

    public:
        void Commit(StepPtr step);
        std::vector<StepPtr> NextSteps();
        bool CheckEdge();
        bool IsComplete();
        void Materialize(EdgeId edge_id) &&;

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
    inline bool EdgeCrawler<StepMaxSize, SubPathStepsSize>::IsComplete() {
        // Check that tree path contains more than 1 step and last step is port (contains port point)
        return path_position_->GetDepth() > 2 && path_position_->IsPort();
    }

    template <size_t StepMaxSize, size_t SubPathStepsSize>
    inline void EdgeCrawler<StepMaxSize, SubPathStepsSize>::Commit(StepPtr step) {
        StepTreeNodePtr next_node = path_position_->MakeChild(step); // std::make_shared<PathNode>(step, path_position_);
        path_position_ = next_node;
    }

    template <size_t StepMaxSize, size_t SubPathStepsSize>
    inline bool EdgeCrawler<StepMaxSize, SubPathStepsSize>::CheckEdge() {
        // TODO: to be implemented
        // Check that current edge path is valid
        return true;
    }

    template <size_t StepMaxSize, size_t SubPathStepsSize>
    inline void EdgeCrawler<StepMaxSize, SubPathStepsSize>::Materialize(EdgeId edge_id) && {
        // TODO: to be implemented
        throw std::runtime_error{"Materialize not implemented"};
    }
}