#include <unordered_map>

#include <optical_graph_recognition.h>
#include <utils/disjoint_set.h>
#include <utils/stack_vector.h>

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
        using SetPtr = std::unique_ptr<utils::DisjointSet>;
        std::unordered_map<point::Point*, SetPtr> points_mapping;
        size_t vertex_id_counter = 0;
        iterator::Neighbourhood8 neighbourhood;

        auto process_vertex_point = [&](const point::PointPtr& point) {
            utils::StackVector<utils::DisjointSet*, 9> sets;
            for (const point::PointPtr& neighbour : neighbourhood(point, grm_)) {
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

        utils::ForAll(grm_, [&](point::PointPtr& point) {
            if (is_vertex(point)) {
                process_vertex_point(point);
            }
        });

        VertexId next_vertex_id = 0;
        using SetId = size_t;
        std::unordered_map<SetId, VertexId> id_mapping;
        std::unordered_map<VertexId, VertexPtr> vertexes;

        utils::ForAll(grm_, [&](point::PointPtr& point) {
            if (!points_mapping.contains(point.get())) {
                return;
            }

            SetPtr set = std::move(points_mapping[point.get()]);
            SetId set_id = set->GetId();
            if (!id_mapping.contains(set_id)) {
                id_mapping[set_id] = next_vertex_id;
                next_vertex_id++;
            }
            VertexId vid = id_mapping[set_id];

            if (!vertexes.contains(vid)) {
                vertexes[vid] = std::make_shared<Vertex>(vid);
            }

            std::shared_ptr<point::VertexPoint> vpoint = std::make_shared<point::VertexPoint>(point->row, point->column);
            point = vpoint;
            vertexes[vid]->points.push_back(vpoint);
            vpoint->vertex = vertexes[vid];
        });
    }
}
