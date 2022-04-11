#pragma once

#include <memory>

namespace ogr {
    struct Vertex;
    struct Edge;
}

namespace ogr::point {
    struct IGraphRecognitionPoint;
    struct FilledPoint;

    using Point = IGraphRecognitionPoint;
    using PointPtr = std::shared_ptr<Point>;
    using WeakPointPtr = std::weak_ptr<Point>;

    using FilledPointPtr = std::shared_ptr<FilledPoint>;

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
    };

    struct EdgePoint : FilledPoint {
        using FilledPoint::FilledPoint;
        std::weak_ptr<Edge> edge;
    };
}