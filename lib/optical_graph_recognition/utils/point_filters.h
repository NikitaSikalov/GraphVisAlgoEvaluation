#pragma once

#include <ogr_components/point.h>

namespace ogr::utils {
    struct IPointFilter {
        virtual point::PointPtr operator()(point::PointPtr point) const = 0;
        virtual ~IPointFilter() = default;
    };

    struct IdentityPointFilter : IPointFilter {
        point::PointPtr operator()(point::PointPtr point) const override {
            return point;
        }
    };

    struct EdgePointFilterWithSourceVertex : IPointFilter {
        VertexId source;

        explicit EdgePointFilterWithSourceVertex(VertexId vid) : source(vid) {}

        point::PointPtr operator()(point::PointPtr point) const override {
            if (!point::IsEdgePoint(point)) {
                return point;
            }

            point::EdgePointPtr edge_point = std::dynamic_pointer_cast<point::EdgePoint>(point);
            for (const std::weak_ptr<Edge> edge : edge_point->edges) {
                if (edge.lock()->v1 == source) {
                    return point;
                }
            }

            return std::make_shared<point::FilledPoint>(point->row, point->column);
        }
    };

    struct EdgePointFilterWithVertex : IPointFilter {
        VertexId vertex_id;

        explicit EdgePointFilterWithVertex(VertexId vid) : vertex_id(vid) {}

        point::PointPtr operator()(point::PointPtr point) const override {
            if (!point::IsEdgePoint(point)) {
                return point;
            }

            point::EdgePointPtr edge_point = std::dynamic_pointer_cast<point::EdgePoint>(point);
            for (const std::weak_ptr<Edge> edge : edge_point->edges) {
                if (edge.lock()->v1 == vertex_id || edge.lock()->v2 == vertex_id) {
                    return point;
                }
            }

            return std::make_shared<point::FilledPoint>(point->row, point->column);
        }
    };

    struct EdgePointFilter : IPointFilter {
        EdgeId edge_id;

        explicit EdgePointFilter(EdgeId eid) : edge_id(eid) {}

        point::PointPtr operator()(point::PointPtr point) const override {
            if (!point::IsEdgePoint(point)) {
                return point;
            }

            point::EdgePointPtr edge_point = std::dynamic_pointer_cast<point::EdgePoint>(point);
            for (const std::weak_ptr<Edge> edge : edge_point->edges) {
                if (edge.lock()->id == edge_id) {
                    return point;
                }
            }

            return std::make_shared<point::FilledPoint>(point->row, point->column);
        }
    };
}
