#pragma once

#include <crawler/step_tree_node.h>
#include <ogr_components/matrix.h>

#include <vector>

namespace ogr::crawler {
    template <size_t StepMaxSize, size_t SubPathStepsSize>
    class EdgeCrawler {
        using PathNode = StepTreeNode<SubPathStepsSize>;
    public:
        void Commit(StepPtr step);
        std::vector<StepPtr> NextSteps();
        bool CheckEdge();
        bool IsComplete();
        void Materialize() &&;
    private:
        std::shared_ptr<PathNode> path_position_;
        std::shared_ptr<matrix::Grm> grm_;
    };
}