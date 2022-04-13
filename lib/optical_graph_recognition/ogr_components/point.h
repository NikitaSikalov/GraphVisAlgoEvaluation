#pragma once

#include <iostream>
#include <memory>

#include <utils/types.h>
#include <utils/geometry.h>

namespace ogr {
    struct Vertex;
    struct Edge;
}

namespace ogr::point {
    struct IGraphRecognitionPoint;
    struct FilledPoint;
    struct VertexPoint;

    using Point = IGraphRecognitionPoint;
    using PointPtr = std::shared_ptr<Point>;
    using WeakPointPtr = std::weak_ptr<Point>;

    using FilledPointPtr = std::shared_ptr<FilledPoint>;
    using VertexPointPtr = std::shared_ptr<VertexPoint>;

    struct IGraphRecognitionPoint {
        const size_t row;
        const size_t column;

        IGraphRecognitionPoint(const size_t row, const size_t column) : row(row), column(column) {}

        virtual bool IsEmpty() const = 0;
        virtual PointPtr Clone() const = 0;

        virtual ~IGraphRecognitionPoint() = default;
    };

    struct FilledPoint : IGraphRecognitionPoint, std::enable_shared_from_this<FilledPoint> {
        using IGraphRecognitionPoint::IGraphRecognitionPoint;

        bool IsEmpty() const override {
            return false;
        }

        bool IsMarked() const {
            return marked_;
        }

        void Mark() {
            marked_ = true;
        }

        void ResetMark() {
            marked_ = false;
        }

        PointPtr Clone() const override {
            FilledPointPtr filled_point = std::make_shared<FilledPoint>(*this);
            return filled_point;
        }

    private:
        bool marked_{false};
    };

    struct EmptyPoint : IGraphRecognitionPoint {
        using IGraphRecognitionPoint::IGraphRecognitionPoint;

        bool IsEmpty() const override {
            return true;
        }

        PointPtr Clone() const override {
            std::shared_ptr<EmptyPoint> point = std::make_shared<EmptyPoint>(*this);
            return point;
        }
    };

    struct VertexPoint : FilledPoint {
        using FilledPoint::FilledPoint;
        std::weak_ptr<Vertex> vertex;
        bool is_port_point{false};

        PointPtr Clone() const override {
            VertexPointPtr point = std::make_shared<VertexPoint>(*this);
            return point;
        }
    };

    struct EdgePoint : FilledPoint {
        using FilledPoint::FilledPoint;
        std::weak_ptr<Edge> edge;

        PointPtr Clone() const override {
            std::shared_ptr<EdgePoint> point = std::make_shared<EdgePoint>(*this);
            return point;
        }
    };

    inline utils::PlanarVector operator-(const Point& p1, const Point& p2) {
        return utils::PlanarVector {
            .x = static_cast<double>(p1.column) - p2.column,
            .y = static_cast<double>(p1.row) - p2.row
        };
    }

    inline bool IsVertexPoint(const PointPtr& point) {
        return utils::Is<VertexPoint>(point.get());
    }

    inline bool IsEdgePoint(const PointPtr& point) {
        return utils::Is<EdgePoint>(point.get());
    }

    inline bool IsMarkedPoint(const PointPtr& point) {
        return utils::As<FilledPoint>(point.get())->IsMarked();
    }

    inline bool IsPortPoint(const PointPtr& point) {
        return utils::Is<VertexPoint>(point.get()) && utils::As<VertexPoint>(point.get())->is_port_point;
    }
}
