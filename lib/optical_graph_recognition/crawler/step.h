#pragma once

#include <ogr_components/point.h>
#include <ogr_components/matrix.h>
#include <iterators/consecutive_iterator.h>
#include <iterators/neighbours.h>
#include <utils/geometry.h>
#include <utils/stack_vector.h>
#include <utils/debug.h>

#include <plog/Log.h>

#include <array>
#include <memory>
#include <vector>
#include <exception>
#include <set>

namespace ogr::crawler {
    struct IStep;
    using StepPtr = std::shared_ptr<IStep>;

    /** General interface for steps of all sizes */
    struct IStep {
        virtual void Push(point::FilledPointPtr point) = 0;
        virtual double GetDirectionAngle() const = 0;
        virtual bool IsExhausted() const = 0;
        virtual size_t Size() const  = 0;
        virtual bool IsPort() const = 0;
        virtual point::FilledPointPtr Back() const = 0;
        virtual point::FilledPointPtr Front() const = 0;
        virtual std::vector<point::FilledPointPtr> GetUnmarkedNeighbours(const matrix::Grm& grm) = 0;
        virtual std::vector<point::FilledPointPtr> GetPoints() const = 0;

        virtual ~IStep() = default;
    };

    template <size_t MaxSize>
    class Step final : public IStep {
    public:
        void Push(point::FilledPointPtr point) override {
            if (point::IsPortPoint(point)) {
                is_port_ = true;
            }

            points_.PushBack(point);
        }

        double GetDirectionAngle() const override {
            point::PointPtr p1 = points_.Back();
            point::PointPtr p2 = points_.Front();
            utils::PlanarVector v = *p1 - *p2;
            v.Normalize();
            return utils::Rad2Deg(v.GetAngle());
        }

        bool IsExhausted() const override {
            return points_.Size() == MaxSize;
        }

        size_t Size() const override {
            return points_.Size();
        }

        bool IsPort() const override {
            return is_port_;
        }

        point::FilledPointPtr Back() const override {
            return points_.Back();
        }

        point::FilledPointPtr Front() const override {
            return points_.Front();
        }

        std::vector<point::FilledPointPtr> GetUnmarkedNeighbours(const matrix::Grm& grm) override {
            std::set<point::Point*> used;
            std::vector<point::FilledPointPtr> result;

            for (const auto& point : points_) {
                auto neighbours = neighbourhood_(point, grm);
                neighbours = iterator::filter::FilterMarkedPoints(neighbours);
                for (point::PointPtr& neighbour : neighbours) {
                    if (used.contains(neighbour.get())) {
                        continue;
                    }
                    used.insert(neighbour.get());
                    result.push_back(utils::As<point::FilledPoint>(neighbour));
                }
            }

            return result;
        }

        std::vector<point::FilledPointPtr> GetPoints() const override {
            std::vector<point::FilledPointPtr> result;
            for (point::FilledPointPtr point : points_) {
                result.push_back(point);
            }

            return result;
        }

    private:
        utils::StackVector<point::FilledPointPtr, MaxSize> points_;
        iterator::Neighbourhood8 neighbourhood_;

        // Contains at least one port point
        bool is_port_{false};
    };

    template <size_t StepSize>
    std::vector<StepPtr> MakeSteps(point::FilledPointPtr point, const matrix::Grm& grm) {
        LOG_DEBUG << "Make steps from point: " << debug::DebugDump(*point);

        using NeighbourhoodStrategy = iterator::Neighbourhood8;
        NeighbourhoodStrategy neighbourhood;

        if (point->IsMarked()) {
            throw std::runtime_error{"Point is already marked"};
        }

        iterator::ConsecutivePointsIterator<NeighbourhoodStrategy> walker(grm, point);
        auto next_step = [&]() -> StepPtr {
            LOG_DEBUG << "Building new step";
            StepPtr next_step = std::make_shared<Step<StepSize>>();
            while (auto next_iteration = walker.Next()) {
                point::FilledPointPtr point = utils::As<point::FilledPoint>(*next_iteration);
                point->Mark();

                LOG_DEBUG << "Add to step point: " << debug::DebugDump(*point);

                next_step->Push(point);
                if (next_step->IsExhausted()) {
                    break;
                }
            }

            LOG_DEBUG << "Step is built";
            LOG_DEBUG << debug::DebugDump(*next_step);
            return next_step;
        };

        std::vector<StepPtr> steps;
        while (true) {
            steps.push_back(next_step());
            if (!walker.ResetToOtherPath()) {
                return steps;
            }
        }
    }

    template <size_t StepSize, typename TStackVector>
    std::vector<StepPtr> MakeSteps(TStackVector& points, const matrix::Grm& grm) {
        std::vector<StepPtr> result;
        for (point::FilledPointPtr& point : points) {
            if (point->IsMarked()) {
                continue;
            }
            std::vector<StepPtr> next_steps = MakeSteps<StepSize>(point, grm);
            for (StepPtr& step : next_steps) {
                result.push_back(step);
            }
        }

        return result;
    }

}