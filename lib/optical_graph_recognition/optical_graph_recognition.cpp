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
        std::unordered_map<point::Point*, SetPtr> vertexes_sets;
        size_t vertex_id_counter = 0;
        iterator::Neighbourhood8 neighbourhood;

        auto process_vertex_point = [&](const point::PointPtr& point) {
            utils::StackVector<utils::DisjointSet*, 9> sets;
            for (const point::PointPtr& neighbour : neighbourhood(point, grm_)) {
                if (vertexes_sets.contains(neighbour.get())) {
                    sets.PushBack(vertexes_sets[neighbour.get()].get());
                }
            }

            SetPtr new_set = std::make_unique<utils::DisjointSet>(vertex_id_counter++);

            if (!sets.Empty()) {
                sets.PushBack(new_set.get());
                utils::MergeDisjointSets(sets);
            }

            vertexes_sets[point.get()] = std::move(new_set);
        };

        for (const utils::Row<point::PointPtr>& row : grm_) {
            for (const point::PointPtr& point : row) {
                if (is_vertex(point)) {
                    process_vertex_point(point);
                }
            }
        }
    }
}
