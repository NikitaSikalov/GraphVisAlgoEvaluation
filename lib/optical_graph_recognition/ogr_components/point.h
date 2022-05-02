#pragma once

#include <utils/types.h>
#include <utils/geometry.h>

#include <iostream>
#include <memory>
#include <set>


namespace ogr {
    struct Vertex;
    struct Edge;
}

namespace ogr::point {
    struct IGraphRecognitionPoint;
    struct FilledPoint;
    struct VertexPoint;
    struct EdgePoint;

    using Point = IGraphRecognitionPoint;
    using PointPtr = std::shared_ptr<Point>;
    using WeakPointPtr = std::weak_ptr<Point>;

    using FilledPointPtr = std::shared_ptr<FilledPoint>;
    using VertexPointPtr = std::shared_ptr<VertexPoint>;

    using EdgePointPtr = std::shared_ptr<EdgePoint>;
    using EdgePointWeakPtr = std::weak_ptr<EdgePoint>;

    struct IGraphRecognitionPoint {
        const size_t row;
        const size_t column;

        IGraphRecognitionPoint(const size_t row, const size_t column) : row(row), column(column) {}

        virtual bool IsEmpty() const = 0;
        virtual ~IGraphRecognitionPoint() = default;
    };

    struct FilledPoint : IGraphRecognitionPoint {
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

    private:
        bool marked_{false};
    };

    struct EmptyPoint : IGraphRecognitionPoint {
        using IGraphRecognitionPoint::IGraphRecognitionPoint;

        bool IsEmpty() const override {
            return true;
        }
    };

    struct VertexPoint : FilledPoint {
        using FilledPoint::FilledPoint;
        std::weak_ptr<Vertex> vertex;
        bool is_port_point{false};
    };

    struct EdgePoint : FilledPoint {
        using FilledPoint::FilledPoint;

        std::vector<std::weak_ptr<Edge>> edges;
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
        if (FilledPointPtr filled_point = std::dynamic_pointer_cast<FilledPoint>(point)) {
            return filled_point->IsMarked();
        }

        return false;
    }

    inline bool IsPortPoint(const PointPtr& point) {
        return utils::Is<VertexPoint>(point.get()) && utils::As<VertexPoint>(point.get())->is_port_point;
    }

    inline bool IsFilledPoint(const PointPtr& point) {
        return utils::Is<FilledPoint>(point.get());
    }

    inline void Unmark(const PointPtr& point) {
        return utils::As<FilledPoint>(point.get())->ResetMark();
    }

    inline void Mark(const PointPtr& point) {
        return utils::As<FilledPoint>(point.get())->Mark();
    }
}
