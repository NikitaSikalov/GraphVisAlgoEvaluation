#include "optical_graph_recognition.h"

#include <utils/disjoint_set.h>
#include <utils/stack_vector.h>
#include <utils/debug.h>
#include <crawler/edges_detector.h>

#include <plog/Log.h>

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

        using SetPtr = std::unique_ptr<utils::DisjointSet>;
        std::unordered_map<point::Point *, SetPtr> points_mapping;
        size_t vertex_id_counter = 0;
        iterator::Neighbourhood8 neighbourhood;

        auto process_vertex_point = [&](const point::PointPtr &point) {
            utils::StackVector<utils::DisjointSet *, 9> sets;
            for (const point::PointPtr &neighbour: neighbourhood(point, grm_)) {
                if (points_mapping.contains(neighbour.get())) {
                    sets.PushBack(points_mapping[neighbour.get()].get());
                }
            }

            SetPtr new_set = std::make_unique<utils::DisjointSet>(vertex_id_counter++);

            if (!sets.Empty()) {
                sets.PushBack(new_set.get());
                utils::MergeDisjointSets(sets);
            }

            points_mapping[point.get()] = std::move(new_set);
        };

        LOG_DEBUG << "Building vertexes sets points...";

        utils::ForAll(grm_, [&](point::PointPtr &point) {
            // Consider only filled points
            if (point->IsEmpty()) {
                return;
            }

            if (is_vertex(point)) {
                process_vertex_point(point);
            }
        });

        VertexId next_vertex_id = 0;
        using SetId = size_t;
        std::unordered_map<SetId, VertexId> id_mapping;

        LOG_DEBUG << "Building vertexes objects from vertex points";

        utils::ForAll(grm_, [&](point::PointPtr &point) {
            if (!points_mapping.contains(point.get())) {
                return;
            }

            LOG_DEBUG << "Process vertex point: " << debug::DebugDump(*point);

            SetId set_id = points_mapping[point.get()]->GetId();

            if (!id_mapping.contains(set_id)) {
                id_mapping[set_id] = next_vertex_id;
                next_vertex_id++;
            }
            VertexId vid = id_mapping[set_id];

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

        debug::DebugDump(grm_, true);
        size_t edge_id_counter = 0;

        for (const auto&[_, vertex]: vertexes_) {
            // Useful for debugging
            if (vertex_id.has_value() && vertex->id != *vertex_id) {
                continue;
            }

            LOG_INFO << "Detect edges for vertex with id = " << vertex->id;

            auto found_edges = crawler::FindEdges(*vertex, grm_, edge_id_counter);

            for (const EdgePtr& edge : found_edges) {
                edges_[edge->id] = edge;
            }

            debug::DebugDump(grm_, /*force*/false, vertex->id);

            utils::ForAll(grm_, [](const point::PointPtr& point) {
                if (point::IsFilledPoint(point)) {
                    point::Unmark(point);
                }

                if (point::IsVertexPoint(point) && !point::IsPortPoint(point)) {
                    point::Mark(point);
                }
            });

            debug::DebugDump(grm_, /*force*/true, vertex->id);
        }

    }
}
