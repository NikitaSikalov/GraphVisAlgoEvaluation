#pragma once

#include <ogr_components/point.h>

namespace ogr {
    using VertexId = size_t;
    using EdgeId = size_t;

    struct Vertex {
        VertexId id;
    };

    struct Edge {
        EdgeId id;
        VertexId v1;
        VertexId v2;
    };

    using VertexPtr = std::shared_ptr<Vertex>;
    using EdgePtr = std::shared_ptr<Edge>;
}