#pragma once

#include <ogr_components/point.h>

namespace ogr {
    using VertexId = size_t;
    using EdgeId = size_t;

    struct Vertex {
        VertexId id;
        std::vector<std::weak_ptr<point::VertexPoint>> points;

        explicit Vertex(const VertexId vid) : id(vid) {}
    };

    struct Edge {
        EdgeId id;
        VertexId v1;
        VertexId v2;
        std::vector<std::weak_ptr<point::EdgePoint>> points;

        Edge(const EdgeId eid, const VertexId vid1, const VertexId vid2) : id(eid), v1(vid1), v2(vid2) {}
    };

    using VertexPtr = std::shared_ptr<Vertex>;
    using EdgePtr = std::shared_ptr<Edge>;
}