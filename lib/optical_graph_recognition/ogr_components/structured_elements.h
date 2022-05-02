#pragma once

#include <ogr_components/point.h>

namespace ogr {
    struct Vertex;
    struct Edge;

    using VertexId = size_t;
    using EdgeId = size_t;

    using VertexPtr = std::shared_ptr<Vertex>;
    using EdgePtr = std::shared_ptr<Edge>;

    struct Vertex {
        VertexId id;
        std::vector<std::weak_ptr<point::VertexPoint>> points;
        std::vector<std::weak_ptr<point::VertexPoint>> port_points;

        explicit Vertex(const VertexId vid) : id(vid) {}
    };

    struct Edge {
        EdgeId id;
        VertexId v1;
        VertexId v2;
        std::vector<std::weak_ptr<point::EdgePoint>> points;
        double irregularity = 0;

        Edge(const EdgeId eid, const VertexId vid1, const VertexId vid2) : id(eid), v1(vid1), v2(vid2) {}

        void Reset() {
            for (auto& weak_point : points) {
                point::EdgePointPtr point = weak_point.lock();
                if (!point) {
                    throw std::runtime_error{"Cant lock weak pointer to edge point"};
                }

                std::vector<std::weak_ptr<Edge>> next_edges;

                bool edge_removed = false;
                for (auto& weak_edge : point->edges) {
                    if (weak_edge.lock()->id == id) {
                        edge_removed = true;
                        continue;
                    }
                    next_edges.push_back(weak_edge);
                }

                if (!edge_removed) {
                    throw std::runtime_error{"Invalid edge point"};
                }

                point->edges = std::move(next_edges);
            }
        }
    };

    namespace point {
        inline std::set<EdgeId> GetEdgesSet(point::EdgePointPtr edge_point) {
            std::set<EdgeId> edges;

            for (auto edge_weak_ptr : edge_point->edges) {
                EdgePtr edge_ptr = edge_weak_ptr.lock();
                edges.insert(edge_ptr->id);
            }

            return edges;
        }
    }
}