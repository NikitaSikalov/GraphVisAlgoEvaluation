#pragma once

#include <utils/types.h>
#include <utils/stack_vector.h>

namespace ogr::utils {
    template <typename T>
    inline Matrix<T> MakeMatrix(const size_t rows, const size_t columns) {
        return std::vector<std::vector<T>>(rows, std::vector<T>(columns));
    }

    template <typename T>
    inline Matrix<T> MakeSquareMatrix(const size_t size) {
        return MakeMatrix<T>(size, size);
    }

    template <typename TStackVector, typename TItem>
    inline auto MapMatrixItems(const TStackVector& coords, const Matrix<TItem>& matrix) {
        utils::StackVector<point::PointPtr, TStackVector::kMaxSize> result;
        for (const auto& [row, column] : coords) {
            result.PushBack(matrix[row][column]);
        }

        return result;
    }
}