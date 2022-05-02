#include "optical_graph_recognition.h"

#include <utils/stack_vector.h>
#include <utils/debug.h>
#include <crawler/edges_detector.h>
#include <algo_utils/gluer.h>
#include <map/composite_map.h>
#include <map/reset_decorator.h>

#include <plog/Log.h>

#include <string>


namespace std {
    template <>
    struct hash<std::pair<ogr::VertexId, ogr::VertexId>> {
        size_t operator()(const std::pair<ogr::VertexId, ogr::VertexId>& key) const {
            auto hashFn = std::hash<ogr::VertexId>{};
            uint64_t hash1 = hashFn(key.first);
            uint64_t hash2 = hashFn(key.second);
            return hash1 ^ hash2;
        }
    };
}

namespace ogr {
    namespace {
        matrix::Grm MakeGraphRecognitionMatrixFromCvMatrix(const cv::Mat& image) {
            const size_t rows = image.rows;
            const size_t columns = image.cols;
            matrix::Grm grm = matrix::MakeGraphRecognitionMatrix(rows, columns);

            for (size_t row = 0; row < rows; ++row) {
                for (size_t column = 0; column < columns; ++column) {
                    const cv::Point cv_point(column, row);
                    const uint8_t value = image.at<uint8_t>(cv_point);
                    if (value != 0) {
                        grm[row][column] = std::make_shared<point::FilledPoint>(row, column);
                    }
                }
            }

            return grm;
        }
    }

    OpticalGraphRecognition::OpticalGraphRecognition(const cv::Mat &source_graph) : grm_(
            MakeGraphRecognitionMatrixFromCvMatrix(source_graph)) {
    }

    void OpticalGraphRecognition::DetectVertexes(std::function<bool(point::PointPtr)> is_vertex) {
        LOG_DEBUG << "Detect vertexes process start";

        algo::PointsGluer<iterator::Neighbourhood8> gluer(grm_);

        utils::ForAll(grm_, [&](point::PointPtr &point) {
            // Consider only filled points
            if (point->IsEmpty()) {
                return;
            }

            if (is_vertex(point)) {
                gluer.AddPoint(point);
            }
        });

        LOG_DEBUG << "Building vertexes objects from vertex points";

        utils::ForAll(grm_, [&](point::PointPtr &point) {
            if (!gluer.ContainsPoint(point)) {
                return;
            }

            LOG_DEBUG << "Process vertex point: " << debug::DebugDump(*point);

            VertexId vid = gluer.GetGroupId(point);

            if (!vertexes_.contains(vid)) {
                vertexes_[vid] = std::make_shared<Vertex>(vid);
            }

            point::VertexPointPtr vpoint = std::make_shared<point::VertexPoint>(point->row, point->column);

            // Mark vertex point, to avoid iterator iterate over these points
            vpoint->Mark();

            point = vpoint;
            vertexes_[vid]->points.push_back(vpoint);
            vpoint->vertex = vertexes_[vid];
        });

        LOG_DEBUG << "End vertexes detecting";

        DetectPortPoints();
    }

    const std::unordered_map<VertexId, VertexPtr>& OpticalGraphRecognition::GetVertexes() const {
        return vertexes_;
    }

    const std::unordered_map<EdgeId, EdgePtr>& OpticalGraphRecognition::GetEdges() const {
        return edges_;
    }

    void OpticalGraphRecognition::DetectPortPoints() {
        for (auto& [_, vertex] : vertexes_) {
            for (auto& point : vertex->points) {
                point::VertexPointPtr vertex_point = point.lock();
                iterator::Neighbourhood8 neighbourhood;
                auto neighbours = neighbourhood(vertex_point, grm_);
                neighbours = iterator::filter::FilterVertexPoints(neighbours);
                if (neighbours.Empty()) {
                    continue;
                }

                vertex_point->ResetMark();
                vertex_point->is_port_point = true;
                vertex->port_points.push_back(vertex_point);
            }
        }
    }

    void OpticalGraphRecognition::DetectEdges(std::optional<VertexId> vertex_id) {
        LOG_DEBUG << "Start detect edges";

        debug::DebugDump(grm_);
        size_t edge_id_counter = 0;

        for (const auto&[_, vertex]: vertexes_) {
            // Useful for debugging
            if (vertex_id.has_value() && vertex->id != *vertex_id) {
                continue;
            }

            LOG_INFO << "Detect edges for vertex with id = " << vertex->id;

            std::vector<EdgePtr> found_edges = crawler::FindEdges(*vertex, grm_, edge_id_counter);

            for (const EdgePtr edge : found_edges) {
                LOG_INFO << "Found edge with id = " << edge->id << " source vertex = " << edge->v1 << " sink vertex = " << edge->v2;
                edges_[edge->id] = edge;
            }

            debug::DebugDump(grm_, vertex->id);

            utils::ForAll(grm_, [](const point::PointPtr& point) {
                if (point::IsFilledPoint(point)) {
                    point::Unmark(point);
                }

                if (point::IsVertexPoint(point) && !point::IsPortPoint(point)) {
                    point::Mark(point);
                }
            });

            debug::DebugDump(grm_, vertex->id);
        }
    }

    void OpticalGraphRecognition::UnionFoundEdges() {
        PostProcessEdges(false);
    }

    void OpticalGraphRecognition::IntersectFoundEdges() {
        PostProcessEdges(true);
    }

    void OpticalGraphRecognition::PostProcessEdges(bool intersect) {
        using EdgeKey = std::pair<VertexId, VertexId>;
        auto make_mirror_key = [](EdgeKey key) -> EdgeKey {
            return std::make_pair(key.second, key.first);
        };

        std::unordered_map<EdgeId, EdgePtr> next_edges_;

        // Build edges map (Source, Sink) -> Edge
        std::unordered_map<EdgeKey, EdgePtr> edges_map;
        for (const auto& [_, edge] : edges_) {
            EdgeKey key = std::make_pair(edge->v1, edge->v2);
            if (edges_map.contains(key)) {
                edges_map[key] = ChooseBestEdge(edge, edges_map[key]);
            } else {
                edges_map[key] = edge;
            }
        }

        std::set<EdgeKey> processed_;
        for (auto& [key, edge] : edges_map) {
            if (processed_.contains(key)) {
                continue;
            }

            processed_.insert(key);
            EdgeKey mirrored_key = make_mirror_key(key);
            if (intersect && !edges_map.contains(mirrored_key)) {
                edge->Reset();
                continue;
            }

            if (!edges_map.contains(mirrored_key)) {
                next_edges_[edge->id] = edge;
                continue;
            }

            processed_.insert(mirrored_key);

            EdgePtr edge1 = edge;
            EdgePtr edge2 = edges_map[mirrored_key];

            EdgePtr best_edge = ChooseBestEdge(edge1, edge2);
            next_edges_[best_edge->id] = best_edge;
        }

        edges_ = std::move(next_edges_);
        ClearGrmFromUnusedEdgePoints();
    }

    EdgePtr OpticalGraphRecognition::ChooseBestEdge(EdgePtr e1, EdgePtr e2) {
        if (e1->irregularity < e2->irregularity) {
            LOG_DEBUG << "Reset edge id = " << e2->id;
            e2->Reset();
            return e1;
        }

        LOG_DEBUG << "Reset edge id = " << e1->id;
        e1->Reset();
        return e2;
    }

    void OpticalGraphRecognition::ClearGrmFromUnusedEdgePoints() {
        utils::ForAll(grm_, [](point::PointPtr& point) {
           if (point::IsEdgePoint(point)) {
               point::EdgePointPtr edge_point = std::dynamic_pointer_cast<point::EdgePoint>(point);
               if (edge_point->edges.empty()) {
                   point = std::make_shared<point::FilledPoint>(point->row, point->column);
               }
           }
        });
    }

    void OpticalGraphRecognition::DumpResultImages(const std::filesystem::path output_dir, std::optional<VertexId> filter_vertex) {
        const std::string base_name = "vertex_";
        for (auto& [vid, vertex] : vertexes_) {
            if (filter_vertex.has_value() && vertex->id != *filter_vertex) {
                continue;
            }

            std::filesystem::path image_path = output_dir / (base_name + std::to_string(vid) + ".png");
            cv::Mat mat = opencv::Grm2CvMat(grm_, utils::EdgePointFilterWithVertex{vid});

            LOG_INFO << "Dump detected edges for vertex with id = " << vid;
            cv::imwrite(image_path, mat);
        }
    }

    void OpticalGraphRecognition::BuildEdgeBundlingMap() {
        std::abort(); // Not implemented
    }

    void OpticalGraphRecognition::CalculateEdgesLength() {
        std::abort(); // Not implemented
    }

}
