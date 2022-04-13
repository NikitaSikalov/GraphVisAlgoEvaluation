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
        StepTreeNode() = default;
        explicit StepTreeNode(StepPtr step) : step_(step) {};
        StepTreeNode(StepPtr step, StepTreeNodePtr parent) : step_(step), parent_(parent) {};

        utils::PlanarVector GetDirection();
        StepTreeNodePtr GetPrevSubPathPointer();
        bool IsPort();
        bool IsRoot();

        StepTreeNodePtr GetParent() {
            if (parent_.expired()) {
                throw std::runtime_error{"Parent weak ptr is expired"};
            }
            return parent_.lock();
        }

        IStep& GetStep() {
            return *step_;
        }

        void AddChild(StepTreeNodePtr child) {
            children_.push_back(child);
        }

    private:
        std::vector<StepTreeNodePtr> children_;
        WeakStepTreeNodePtr parent_;
        StepPtr step_{nullptr};
    };

    template <size_t SubPathStepsSize>
    inline bool StepTreeNode<SubPathStepsSize>::IsPort() {
        return step_->IsPort();
    }

    template <size_t SubPathStepsSize>
    inline bool StepTreeNode<SubPathStepsSize>::IsRoot() {
        return !parent_.use_count();
    }
}