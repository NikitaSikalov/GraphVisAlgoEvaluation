#pragma once

#include <crawler/step.h>

#include <memory>
#include <vector>

namespace ogr::crawler {
    struct IStepTreeNode;
    using StepTreeNodePtr = std::shared_ptr<IStepTreeNode>;
    using WeakStepTreeNodePtr = std::weak_ptr<IStepTreeNode>;

    struct IStepTreeNode {
        virtual double GetLastStepAngle() const = 0;
        virtual double GetStateAngle() const = 0;
        virtual double GetDiffAngleWithPrevState() const = 0;
        virtual double GetDiffAngleWithLastStep() const = 0;
        virtual bool IsPort() const = 0;
        virtual bool IsRoot() const = 0;
        virtual StepPtr GetStep() const = 0;
        virtual size_t GetDepth() const = 0;
        virtual std::shared_ptr<IStepTreeNode> MakeChild(StepPtr step) = 0;
        virtual StepTreeNodePtr GetParentNode() const = 0;

        virtual ~IStepTreeNode() = default;
    };

    template <size_t SubPathStepsSize>
    class StepTreeNode : public std::enable_shared_from_this<StepTreeNode<SubPathStepsSize>>, public IStepTreeNode {
    public:
        StepTreeNode() = default;
        explicit StepTreeNode(StepPtr step) : step_(step) {};

    public:
        static StepTreeNodePtr MakeRoot();

    public:
        double GetLastStepAngle() const override;
        double GetStateAngle() const override;
        double GetDiffAngleWithPrevState() const override;
        double GetDiffAngleWithLastStep() const override;
        bool IsPort() const override;
        bool IsRoot() const override;
        StepPtr GetStep() const override;
        StepTreeNodePtr MakeChild(StepPtr step) override;
        size_t GetDepth() const override;
        StepTreeNodePtr GetParentNode() const override;

    private:
        std::vector<StepTreeNodePtr> children_;
        WeakStepTreeNodePtr parent_;
        StepPtr step_{nullptr};
        WeakStepTreeNodePtr prev_state_;
        double angle_{0};
        size_t depth_{0};
    };

    template <size_t SubPathStepsSize>
    inline StepTreeNodePtr StepTreeNode<SubPathStepsSize>::MakeRoot() {
        StepTreeNodePtr root_node = std::make_shared<StepTreeNode>();
        return root_node;
    }

    template <size_t SubPathStepsSize>
    inline bool StepTreeNode<SubPathStepsSize>::IsPort() const {
        return step_->IsPort();
    }

    template <size_t SubPathStepsSize>
    inline bool StepTreeNode<SubPathStepsSize>::IsRoot() const {
        return !parent_.use_count();
    }

    template <size_t SubPathStepsSize>
    inline size_t StepTreeNode<SubPathStepsSize>::GetDepth() const {
        return depth_;
    }

    template <size_t SubPathStepsSize>
    inline StepPtr StepTreeNode<SubPathStepsSize>::GetStep() const {
        return step_;
    }

    template <size_t SubPathStepsSize>
    inline double StepTreeNode<SubPathStepsSize>::GetStateAngle() const {
        return angle_;
    }

    template <size_t SubPathStepsSize>
    inline double StepTreeNode<SubPathStepsSize>::GetLastStepAngle() const {
        if (IsRoot()) {
            return 0;
        }

        return step_->GetDirectionAngle();
    }

    template <size_t SubPathStepsSize>
    inline StepTreeNodePtr StepTreeNode<SubPathStepsSize>::GetParentNode() const {
        return parent_.lock();
    }

    template <size_t SubPathStepsSize>
    inline double StepTreeNode<SubPathStepsSize>::GetDiffAngleWithPrevState() const {
        if (depth_ <= SubPathStepsSize) {
            return 0;
        }

        return utils::AbsDiffAngles(prev_state_.lock()->GetStateAngle(), angle_);
    }

    template <size_t SubPathStepsSize>
    inline double StepTreeNode<SubPathStepsSize>::GetDiffAngleWithLastStep() const {
        if (depth_ < 2) {
            return 0;
        }

        return utils::AbsDiffAngles(parent_.lock()->GetStateAngle(), step_->GetDirectionAngle());
    }

    template <size_t SubPathStepsSize>
    inline StepTreeNodePtr StepTreeNode<SubPathStepsSize>::MakeChild(StepPtr step) {
        std::shared_ptr<StepTreeNode> step_node = std::make_shared<StepTreeNode>(step);
        children_.push_back(step_node);
        step_node->parent_ = this->shared_from_this();
        step_node->depth_ = depth_ + 1;
        if (step_node->depth_ <= SubPathStepsSize) {
            step_node->prev_state_ = IsRoot() ? this->shared_from_this() : prev_state_.lock();
            if (IsRoot()) {
                step_node->angle_ = utils::NormalizeAngle(step->GetDirectionAngle());
                return step_node;
            }
            const double step_angle = utils::AlignAngle(step->GetDirectionAngle(), angle_);
            step_node->angle_ = utils::NormalizeAngle((angle_ * depth_ + step_angle) / (depth_ + 1));
            return step_node;
        }

        StepTreeNodePtr tree_node_ptr = this->shared_from_this();
        for (size_t i = 0; i < SubPathStepsSize - 1; ++i) {
            tree_node_ptr = tree_node_ptr->GetParentNode();
        }
        step_node->prev_state_ = tree_node_ptr;

        if (step->IsPort() && step->Size() == 1) {
            step_node->angle_ = utils::NormalizeAngle(angle_);
            return step_node;
        }

        const double step_actual_angle = step->GetDirectionAngle();
        const double aligned_step_angle = utils::AlignAngle(step_actual_angle, angle_);
        const double last_step_angle = tree_node_ptr->GetLastStepAngle();
        const double aligned_last_step_angle = utils::AlignAngle(last_step_angle, angle_);
        step_node->angle_ = utils::NormalizeAngle(angle_ - (aligned_last_step_angle - aligned_step_angle) / SubPathStepsSize);
        return step_node;
    }
}