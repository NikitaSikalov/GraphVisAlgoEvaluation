#pragma once

#include <ogr_components/point.h>
#include <utils/vector.h>
#include <utils/stack_vector.h>

#include <array>
#include <memory>

namespace ogr::crawler {
    struct IStep;
    using StepPtr = std::shared_ptr<IStep>;

    struct IStep {
        virtual void Push(point::FilledPointPtr point) = 0;
        virtual utils::Vector2 GetDirection() const = 0;
        virtual bool IsExhausted() const = 0;

        virtual ~IStep() = default;
    };

    template <size_t MaxSize>
    class Step final : IStep {
    public:
        void Push(point::FilledPointPtr point) override {

        }

        utils::Vector2 GetDirection() const override {

        }

        bool IsExhausted() const override {
            return points_.Size() == MaxSize;
        }

    private:
        utils::StackVector<point::FilledPointPtr, MaxSize> points_;
    };
}