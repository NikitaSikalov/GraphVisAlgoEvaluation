#pragma once

#include <ogr_components/point.h>
#include <ogr_components/matrix.h>
#include <iterators/consecutive_iterator.h>
#include <iterators/neighbours.h>
#include <utils/vector.h>
#include <utils/stack_vector.h>

#include <array>
#include <memory>
#include <vector>
#include <exception>

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

    template <size_t StepSize, typename TStackVector>
    std::vector<StepPtr> MakeSteps(TStackVector& points, const matrix::Grm& grm) {
        for (point::FilledPointPtr& point : points) {
            if (point->IsMarked()) {
                continue;
            }
            // TODO: continue coding here...
        }

    }

    template <size_t StepSize>
    std::vector<StepPtr> MakeSteps(point::FilledPointPtr point, const matrix::Grm& grm) {
        using NeighbourhoodStrategy = iterator::Neighbourhood8;
        NeighbourhoodStrategy neighbourhood;

        if (point->IsMarked()) {
            throw std::runtime_error{"Point is already marked"};
        }

        auto next_step = [](iterator::ConsecutivePointsIterator<NeighbourhoodStrategy> walker) -> StepPtr {
            StepPtr next_step = std::make_shared<Step<StepSize>>();
            while (auto next_iteration = walker.Next()) {
                point::FilledPointPtr point = utils::As<point::FilledPoint>(*next_iteration);
                point->Mark();
                next_step->Push(point);
                if (next_step->IsExhausted()) {
                    break;
                }
            }

            return next_step;
        };

        std::vector<StepPtr> steps;
        iterator::ConsecutivePointsIterator<NeighbourhoodStrategy> walker(grm, point);
        while (true) {
            steps.push_back(next_step(walker));
            if (!walker.ResetAnotherPath()) {
                return steps;
            }
        }
    }

}