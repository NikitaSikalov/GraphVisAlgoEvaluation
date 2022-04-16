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
            if (Size() <= 1) {
                throw std::runtime_error{"Try to get direction angle from step consists of points less than 2"};
            }

            point::PointPtr p1 = points_.Back();
            point::PointPtr p2 = points_.Front();
            utils::PlanarVector v1 = *p1 - *p2;
            v1.Normalize();
            const double alpha1 = utils::Rad2Deg(v1.GetAngle());
            if (Size() == 2) {
                return alpha1;
            }

            point::PointPtr p3 = points_[Size() - 2];
            utils::PlanarVector v2 = *p1 - *p2;
            v2.Normalize();
            double alpha2 = utils::Rad2Deg(v2.GetAngle());
            alpha2 = utils::AlignAngle(alpha2, alpha1);

            return utils::NormalizeAngle((alpha1 + alpha2) / 2);
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
                    result.push_back(std::dynamic_pointer_cast<point::FilledPoint>(neighbour));
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
                point::FilledPointPtr point = std::dynamic_pointer_cast<point::FilledPoint>(*next_iteration);
                point->Mark();

                LOG_DEBUG << "Add to step point: " << debug::DebugDump(*point);

                next_step->Push(point);
                if (next_step->IsExhausted()) {
                    break;
                }
            }

            LOG_DEBUG << "Step is built: " << debug::DebugDump(*next_step);
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