#pragma once

#include <ogr_components/structured_elements.h>
#include <utils/types.h>
#include <utils/matrix_utils.h>

#include <memory>
#include <exception>
#include <type_traits>

namespace ogr::matrix {
    using GraphRecognitionMatrix = utils::Matrix<point::PointPtr>;
    using Grm = GraphRecognitionMatrix;

    inline Grm MakeGraphRecognitionMatrix(const size_t rows, const size_t columns) {
        utils::Matrix<point::PointPtr> matrix = utils::MakeMatrix<point::PointPtr>(rows, columns);
        for (std::size_t row = 0; row < matrix.size(); ++row) {
            for (std::size_t column = 0; column < matrix[row].size(); ++column) {
                matrix[row][column] = std::make_shared<point::EmptyPoint>(row, column);
            }
        }

        return matrix;
    }

    inline size_t Rows(const Grm& grm) {
        return grm.size();
    }

    inline size_t Columns(const Grm& grm) {
        if (grm.empty()) {
            throw std::runtime_error{"Grm is empty"};
        }

        return grm[0].size();
    }

    inline Grm CopyFromSample(const Grm& sample) {
        Grm result = MakeGraphRecognitionMatrix(Rows(sample), Columns(sample));
        for (size_t row = 0; row < Rows(sample); ++row) {
            for (size_t col = 0; col < Columns(sample); ++col) {
                result[row][col] = sample[row][col]->Clone();
            }
        }

        return result;
    }
}
