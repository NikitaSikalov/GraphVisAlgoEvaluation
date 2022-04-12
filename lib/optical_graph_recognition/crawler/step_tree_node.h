#pragma once

#include <crawler/step.h>

#include <memory>
#include <vector>

namespace ogr::crawler {
    template <size_t SubPathStepsSize>
    class StepTreeNode {
        using StepTreeNodePtr = std::shared_ptr<StepTreeNode>;

        // prevent ref cycles to avoid memory leaks
        using WeakStepTreeNodePtr = std::weak_ptr<StepTreeNode>;

    public:
        explicit StepTreeNode(std::unique_ptr<IStep> step) : step_(std::move(step)) {};
        StepTreeNode(std::unique_ptr<IStep> step, StepTreeNodePtr parent) : step_(std::move(step)), parent_(parent) {

        };

        utils::Vector2 GetDirection();
        StepTreeNodePtr GetPrevSubPathPointer();

    private:
        std::vector<StepTreeNodePtr> children_;
        WeakStepTreeNodePtr parent_{nullptr};
        std::unique_ptr<IStep> step_;
    };
}