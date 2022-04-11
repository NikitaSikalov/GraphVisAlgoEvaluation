#pragma once

#include <memory>
#include <exception>
#include <type_traits>

#include <ogr_components/structured_elements.h>
#include <utils/types.h>
#include <utils/matrix_utils.h>

namespace ogr::matrix {
    using PointPtr = std::shared_ptr<point::IGraphRecognitionPoint>;
    using GraphRecognitionMatrix = utils::Matrix<PointPtr>;
    using Grm = GraphRecognitionMatrix;

    GraphRecognitionMatrix MakeGraphRecognitionMatrix(const size_t rows, const size_t columns) {
        utils::Matrix<PointPtr> matrix = utils::MakeMatrix<PointPtr>(rows, columns);
        for (std::size_t row = 0; row < matrix.size(); ++row) {
            for (std::size_t column = 0; column < matrix[row].size(); ++column) {
                matrix[row][column] = std::make_shared<point::EmptyPoint>(row, column);
            }
        }

        return matrix;
    }

    size_t Rows(const Grm& grm) {
        return grm.size();
    }

    size_t Columns(const Grm& grm) {
        if (grm.empty()) {
            throw std::runtime_error{"Grm is empty"};
        }

        return grm[0].size();
    }
}
